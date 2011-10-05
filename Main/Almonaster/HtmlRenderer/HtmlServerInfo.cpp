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

#include "HtmlRenderer.h"

int HtmlRenderer::WriteServerRules()
{
    int iErrCode, iNumActiveGames, iSystemOptions;
    unsigned int iNumProcessors, iMHz, iNumFiles, iNumPages, iTimeSpent, iNumOpenGames, iNumClosedGames;
    float fValue;
    Seconds iUptime, iCpuTime, sBridierScanFrequency;
    size_t iTotalPhysicalMemory, iTotalFreePhysicalMemory, iTotalSwapMemory, iTotalFreeSwapMemory, iTotalVirtualMemory, stFileCacheSize;

    Variant vDefaultNumUpdatesForClose, vValue, vNumUpdatesDown, vSecondsForLongtermStatus, vNumNukesListed, vAfterWeekendDelay, vUnlimitedEmpirePrivilege;
    char pszDateString [OS::MaxDateLength];
    
    GameConfiguration gcConfig;
    MapConfiguration mcConfig;
    
    HttpServerStatistics stats;
    AlmonasterStatistics aStats;

    UTCTime tNow;
    Time::GetTime(&tNow);
    
    iErrCode = GetGameConfiguration (&gcConfig);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetMapConfiguration (&mcConfig);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::NumUpdatesDownBeforeGameIsKilled, &vNumUpdatesDown);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::SecondsForLongtermStatus, &vSecondsForLongtermStatus);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::AfterWeekendDelay, &vAfterWeekendDelay);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::NumNukesListedInNukeHistories, &vNumNukesListed);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemOptions (&iSystemOptions);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::DefaultNumUpdatesBeforeClose, &vDefaultNumUpdatesForClose);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetBridierTimeBombScanFrequency (&sBridierScanFrequency);
    RETURN_ON_ERROR(iErrCode);
    
    OutputText ("<p><h2>Server information</h2></center><ul><li>The web server is <strong>");
    m_pHttpResponse->WriteText (global.GetHttpServer()->GetServerName());
    OutputText ("</strong>, running at <strong>");
    m_pHttpResponse->WriteText (global.GetHttpServer()->GetIPAddress());
    OutputText ("</strong> on ");

    short httpPort = global.GetHttpServer()->GetHttpPort();
    short httpsPort = global.GetHttpServer()->GetHttpsPort();

    if (httpPort != 0) {
        OutputText ("HTTP port <strong>");
        m_pHttpResponse->WriteText(httpPort);
        OutputText ("</strong>");
    }

    if (httpsPort != 0) {

        if (httpPort != 0)
            OutputText (" and ");

        OutputText ("HTTPS port <strong>");
        m_pHttpResponse->WriteText(httpsPort);
        OutputText ("</strong>");
    }

    unsigned int iNumThreads = global.GetHttpServer()->GetNumThreads();
    OutputText ("</strong></li><li>The web server is using <strong>");
    m_pHttpResponse->WriteText (iNumThreads);
    OutputText ("</strong> thread");
    if (iNumThreads != 1)
        OutputText("s");
    OutputText (" in its thread pool</li>");
    
    iErrCode = OS::GetProcessMemoryStatistics(&iTotalPhysicalMemory, &iTotalVirtualMemory);
    RETURN_ON_ERROR(iErrCode);

    OutputText ("<li>The server process' working set size is <strong>");
    m_pHttpResponse->WriteText ((int64) iTotalPhysicalMemory / 1024); 
    OutputText (" KB</strong> and its virtual memory size is <strong>");
    m_pHttpResponse->WriteText ((int64) iTotalVirtualMemory / 1024); 
    OutputText (" KB</strong></li>");
    
    iErrCode = OS::GetProcessTimeStatistics(&iUptime, &iCpuTime);
    RETURN_ON_ERROR(iErrCode);
    
    OutputText ("<li>The server process has been running for ");
    WriteTime (iUptime);
    OutputText (" and has used ");
    WriteTime (iCpuTime);
    OutputText (" of CPU time</li>");

    iNumFiles = global.GetFileCache()->GetNumFiles();
    stFileCacheSize = global.GetFileCache()->GetSize();
    
    OutputText ("<li>The server file cache contains <strong>");
    m_pHttpResponse->WriteText (iNumFiles);
    OutputText ("</strong> file");
    if (iNumFiles != 1) {
        OutputText ("s");
    }
    if (iNumFiles != 0) {
        OutputText (", totalling <strong>");
        m_pHttpResponse->WriteText ((int64) (stFileCacheSize / 1024));
        OutputText ("</strong> KB");
    }
    OutputText ("</li>");
    
    // Stats
    iErrCode = global.GetHttpServer()->GetStatistics (&stats);
    RETURN_ON_ERROR(iErrCode);
        
    OutputText ("<li>The server has handled <strong>");
    m_pHttpResponse->WriteText (stats.NumRequests);
    OutputText ("</strong> requests today, totalling <strong>");
    m_pHttpResponse->WriteText((int64)stats.NumBytesReceived / 1024);
    OutputText ("</strong> KB received and <strong>");
    m_pHttpResponse->WriteText((int64)stats.NumBytesSent / 1024);
    OutputText ("</strong> KB sent</li>");
    
    aStats = m_sStats;
    iNumPages = aStats.NumPageScriptRenders;
    iTimeSpent = aStats.TotalScriptTime;
    
    OutputText (
        "<li>Since the server was started:<ul>"\
        "<li><strong>"
        );

    m_pHttpResponse->WriteText (iNumPages); 

    OutputText ("</strong> page");
    if (iNumPages != 1) {
        OutputText ("s have");
    } else {
        OutputText (" has");
    }
    
    OutputText (" been rendered");

    if (iNumPages > 0) {

        OutputText (", with an average script time of <strong>");
        m_pHttpResponse->WriteText (iTimeSpent / iNumPages);
        OutputText ("</strong> ms");
    }
        
    OutputText ("</li><li><strong>");
    m_pHttpResponse->WriteText (aStats.Logins);
    OutputText ("</strong> empire");
    if (aStats.Logins != 1) {
        OutputText ("s have");
    } else {
        OutputText (" has");
    }
    OutputText (" logged in</li>");
    
    OutputText ("<li><strong>");
    m_pHttpResponse->WriteText (aStats.EmpiresCreated);
    OutputText ("</strong> empire");
    if (aStats.EmpiresCreated != 1) {
        OutputText ("s have");
    } else {
        OutputText (" has");
    }
    OutputText (" been created</li>");
    
    OutputText ("<li><strong>");
    m_pHttpResponse->WriteText (aStats.EmpiresDeleted);
    OutputText ("</strong> empire");
    if (aStats.EmpiresDeleted != 1) {
        OutputText ("s have");
    } else {
        OutputText (" has");
    }
    OutputText (" been deleted</li>");
    
    OutputText ("<li><strong>");
    m_pHttpResponse->WriteText (aStats.GamesStarted);
    OutputText ("</strong> game");
    if (aStats.GamesStarted != 1) {
        OutputText ("s have");
    } else {
        OutputText (" has");
    }
    OutputText (" been started</li>");
    
    OutputText ("<li><strong>");
    m_pHttpResponse->WriteText (aStats.GamesEnded);
    OutputText ("</strong> game");
    if (aStats.GamesEnded != 1) {
        OutputText ("s have");
    } else {
        OutputText (" has");
    }
    OutputText (" ended</li></ul></li>");
    
    iErrCode = GetNumOpenGames(&iNumOpenGames);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetNumClosedGames(&iNumClosedGames);
    RETURN_ON_ERROR(iErrCode);
        
    iNumActiveGames = iNumOpenGames + iNumClosedGames;
        
    if (iNumActiveGames == 0) {
        OutputText ("<li>No games are active on the server</li>");
    } else {
        OutputText ("<li><strong>");
        if (iNumActiveGames == 1) { 
            OutputText ("1</strong> game is active");
        } else {
            m_pHttpResponse->WriteText (iNumActiveGames);
            OutputText ("</strong> games are active");
        }
        OutputText (" on the server (<strong>");
        m_pHttpResponse->WriteText (iNumOpenGames);
        OutputText ("</strong> open, <strong>");
        m_pHttpResponse->WriteText (iNumClosedGames);
        OutputText ("</strong> closed)</li>");

        unsigned int iNumGamingEmpires = m_siNumGamingEmpires;
        if (Time::GetSecondDifference (tNow, m_stEmpiresInGamesCheck) > 5 * 60)
        {
            // Lock to ensure only one query is made at a time
            m_slockEmpiresInGames.Wait();
            if (Time::GetSecondDifference (tNow, m_stEmpiresInGamesCheck) > 15 * 60)
            {
                m_stEmpiresInGamesCheck = tNow;
                m_slockEmpiresInGames.Signal();

                // People who lose the race will see a stale value until the query finishes...
                iErrCode = GetNumUniqueEmpiresInGames(&iNumGamingEmpires);
                RETURN_ON_ERROR(iErrCode);
                m_siNumGamingEmpires = iNumGamingEmpires;
                    
            } else {

                m_slockEmpiresInGames.Signal();
            }
        }

        if (iNumGamingEmpires == 0) {
            OutputText ("<li>No empires are in games on the server</li>");
        } else {
                
            if (iNumGamingEmpires == 1) {
                OutputText ("<li><strong>1</strong> empire is in games on the server</li>");
            } else {
                OutputText ("<li><strong>");
                m_pHttpResponse->WriteText (iNumGamingEmpires);
                OutputText ("</strong> empires are in games on the server</li>");
            }

            iErrCode = GetNumRecentActiveEmpiresInGames(&iNumGamingEmpires);
            RETURN_ON_ERROR(iErrCode);

            if (iNumGamingEmpires == 0) {
                OutputText ("<li>No empires have played recently on the server</li>");
            }
            else if (iNumGamingEmpires == 1) {
                OutputText ("<li><strong>1</strong> empire has played recently on the server</li>");
            }
            else {
                OutputText ("<li><strong>");
                m_pHttpResponse->WriteText (iNumGamingEmpires);
                OutputText ("</strong> empires have played recently on the server</li>");
            }
        }
    }
    
    // Machine information
    OutputText ("</ul><p><center><h2>Machine information</h2></center><ul>");
    
    iErrCode = Time::GetDateString(pszDateString);
    RETURN_ON_ERROR(iErrCode);
        
    OutputText ("<li>The server's local time is <strong>");
    m_pHttpResponse->WriteText (pszDateString);
    OutputText ("</strong>");
        
    int iBias;
    char pszTimeZone[OS::MaxTimeZoneLength];
        
    if (Time::GetTimeZone (pszTimeZone, &iBias) == OK) {
        OutputText (", <strong>");
        m_pHttpResponse->WriteText (pszTimeZone);
        OutputText ("</strong>");
            
        if (iBias != 0) {
            OutputText (" (<strong>");
            if (iBias < 0) {
                OutputText ("GMT");
                m_pHttpResponse->WriteText (iBias / 60);
            } else {
                OutputText ("GMT+");
                m_pHttpResponse->WriteText (iBias / 60);
            }
            OutputText ("</strong>)");
        }
    }
        
    OutputText ("</li>");
    
    OutputText ("<li>The server machine is running <strong>");
    
    char pszOSVersion[OS::MaxOSVersionLength];
    
    iErrCode = OS::GetOSVersion (pszOSVersion);
    RETURN_ON_ERROR(iErrCode);
    m_pHttpResponse->WriteText (pszOSVersion);
    
    char pszProcessorInformation[OS::MaxProcessorInfoLength];
    
    iErrCode = OS::GetProcessorInformation(pszProcessorInformation, &iNumProcessors, &iMHz);
    RETURN_ON_ERROR(iErrCode);
        
    OutputText ("</strong></li><li>The server machine has <strong>");
    m_pHttpResponse->WriteText (iNumProcessors);
    OutputText (" ");

    if (iMHz != CPU_SPEED_UNAVAILABLE) {
        m_pHttpResponse->WriteText (iMHz);
        OutputText (" MHz ");
    }
    m_pHttpResponse->WriteText (pszProcessorInformation);
    OutputText ("</strong> processor");
        
    if (iNumProcessors != 1) {
        OutputText ("s");
    }
    OutputText ("</li>");
    
    iErrCode = OS::GetMemoryStatistics(&iTotalPhysicalMemory, &iTotalFreePhysicalMemory, &iTotalSwapMemory, &iTotalFreeSwapMemory);
    RETURN_ON_ERROR(iErrCode);
    
    OutputText ("<li>The server machine has <strong>");
    m_pHttpResponse->WriteText ((int64) iTotalPhysicalMemory / 1024);
    OutputText (" KB</strong> of physical memory, of which <strong>");
    m_pHttpResponse->WriteText ((int64) (iTotalPhysicalMemory - iTotalFreePhysicalMemory) / 1024);
    OutputText (" KB</strong> are in use</li><li>The server machine has <strong>");
    m_pHttpResponse->WriteText ((int64) iTotalSwapMemory / 1024);
    OutputText (" KB</strong> of swap memory, of which <strong>");
    m_pHttpResponse->WriteText ((int64) (iTotalSwapMemory - iTotalFreeSwapMemory) / 1024);
    OutputText (" KB</strong> are in use</li>");
    
    OutputText ("</ul><p><center><h2>Ships</h2></center><ul><li>Ships are not destroyed "\
        "if they perform special actions at costs that are beneath their full capacities</li>"\
        "<li>Engineers lose <strong>");
    
    m_pHttpResponse->WriteText (gcConfig.fEngineerLinkCost); 
    OutputText ("</strong> BR and MaxBR when they open or close a link</li><li>Stargates lose <strong>");
    m_pHttpResponse->WriteText (gcConfig.fStargateGateCost); 
    OutputText ("</strong> BR and MaxBR when they stargate at least one ship</li>");
    
    if (gcConfig.iShipBehavior & STARGATE_LIMIT_RANGE) {
        
        OutputText ("<li>Stargates can stargate ships to planets up BR / <strong>");
        m_pHttpResponse->WriteText (gcConfig.fJumpgateRangeFactor);
        OutputText ("</strong> hops away </li>");
        
    } else {
        
        OutputText ("<li>Stargate range limitations are <strong>not</strong> enforced</li>");
    }
    
    OutputText ("<li>Colonies cost ");
    
    if (!(gcConfig.iShipBehavior & COLONY_USE_MULTIPLIED_BUILD_COST)) {
        
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (gcConfig.iColonySimpleBuildFactor);
        
    } else {
        
        OutputText ("BR * <strong>");
        m_pHttpResponse->WriteText (gcConfig.fColonyMultipliedBuildFactor);
    }

    OutputText ("</strong> population unit");
    if (gcConfig.iColonySimpleBuildFactor != 1) {
        OutputText ("s");
    }
    OutputText (" each to build</li>");
    
    OutputText ("<li>Colonies deposit up to their BR");
    if (gcConfig.iShipBehavior & COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT) {
        OutputText (" * <strong>");
        m_pHttpResponse->WriteText (gcConfig.fColonyMultipliedDepositFactor);
    } else {
        OutputText (" ^ <strong>");
        m_pHttpResponse->WriteText (gcConfig.fColonyExponentialDepositFactor);
    }
    OutputText ("</strong> population units when colonizing planets</li>"\
        "<li>Colonies can also settle (deposit population units upon) already colonized planets</li>"\
        "<li>Terraformers can increase a planet's agriculture by up to their BR * <strong>");
    
    m_pHttpResponse->WriteText (gcConfig.fTerraformerPlowFactor);
    OutputText ("</strong> agriculture units</li>"\
        "<li>Multiple terraformers can act upon a planet at the same time</li>"\
        "<li>Terraformers can act upon any planet, even those uncolonized or belonging to an enemy"\
        "<li>Troopships can invade a planet with up to BR * <strong>");
    
    m_pHttpResponse->WriteText (gcConfig.fTroopshipInvasionFactor); 
    OutputText ("</strong> population units</li>");
    
    OutputText ("<li>Troopships that fail to invade destroy BR * <strong>");
    m_pHttpResponse->WriteText (gcConfig.fTroopshipFailureFactor);
    OutputText ("</strong> of the planet's population units</li>"\
        "<li>Troopships that successfully invade destroy <strong>");
    
    m_pHttpResponse->WriteText (gcConfig.fTroopshipSuccessFactor * (float) 100);
    OutputText ("%</strong> of the planet's population units</li>"\
        "<li>Cloakers are automatically <strong>");
    
    if (!(gcConfig.iShipBehavior & CLOAKER_CLOAK_ON_BUILD)) {
        OutputText ("un");
    }
    OutputText ("cloaked</strong> when built</li><li>Morphers are automatically <strong>");
    
    if (!(gcConfig.iShipBehavior & MORPHER_CLOAK_ON_CLOAKER_MORPH)) {
        OutputText ("un");
    }
    OutputText ("cloaked</strong> when they morph into cloakers</li>"\
        "<li>Planets annihilated by a doomsday will remain quarantined for BR * <strong>");
    m_pHttpResponse->WriteText (gcConfig.fDoomsdayAnnihilationFactor);
    OutputText ("</strong> updates and will have their agriculture level reduced to zero</li>");
    
    OutputText ("<li>Carriers lose <strong>");
    m_pHttpResponse->WriteText (gcConfig.fCarrierCost);
    OutputText ("</strong> BR and MaxBR when involved in a fleet battle</li>"\
        
        "<li>Builders require <strong>");
    m_pHttpResponse->WriteText (gcConfig.fBuilderMinBR);
    OutputText ("</strong> BR to create a planet</li>"\

        "<li>Builders create planets with resources allocated according to the following formula: <strong>");
    m_pHttpResponse->WriteText (gcConfig.fBuilderMultiplier);
    OutputText ("</strong> * (average planet in game) * ((BR - <strong>");
    m_pHttpResponse->WriteText (gcConfig.fBuilderBRDampener);
    OutputText ("</strong> + <strong>1</strong>) / BR) ^ <strong>2</strong></li>"\

        "<li>Morphers lose <strong>");
    m_pHttpResponse->WriteText (gcConfig.fMorpherCost);
    OutputText ("</strong> BR and MaxBR when they morph to a different tech type</li>"\
        
        "<li>Jumpgates lose <strong>");
    m_pHttpResponse->WriteText (gcConfig.fJumpgateGateCost);
    OutputText ("</strong> BR and MaxBR when they jumpgate at least one ship</li>");
    
    if (gcConfig.iShipBehavior & JUMPGATE_LIMIT_RANGE) {
        
        OutputText ("<li>Jumpgates can jumpgate ships to planets up to BR / <strong>");
        m_pHttpResponse->WriteText (gcConfig.fJumpgateRangeFactor);
        OutputText ("</strong> hops away </li>");
        
    } else {
        
        OutputText ("<li>Jumpgate range limitations are <strong>not</strong> enforced</li>");
    }
    
    OutputText ("<li>Terraforming non-owned planets is <strong>");
    if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {
        OutputText ("disabled");
    } else {
        OutputText ("enabled");
    }
    
    OutputText ("</strong></li><li>Terraformer survival after terraforming is <strong>");
    if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_SURVIVAL) {
        OutputText ("disabled");
    } else {
        OutputText ("enabled");
    }
    
    OutputText ("</strong></li><li>Troopship survival after invading is <strong>");
    if (gcConfig.iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL) {
        OutputText ("disabled");
    } else {
        OutputText ("enabled");
    }
    
    OutputText ("</strong></li><li>Minefield detonation is <strong>");
    if (gcConfig.iShipBehavior & MINEFIELD_DISABLE_DETONATE) {
        OutputText ("disabled");
    } else {
        OutputText ("enabled");
    }

    // Ship costs
    OutputText (
        "</strong></li></ul><p><center><h2>Ship costs</h2>"\
        "<table border=\"1\">"\
        "<tr>"\
            "<th>Ship</th>"\
            "<th>Build Cost</th>"\
            "<th>Maintenance Cost</th>"\
            "<th>Fuel Cost</th>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Attack</b></td>"\
            "<td>(BR + 4 ) ^ 2</td>"\
            "<td>BR * 2</td>"\
            "<td>BR * 4</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Science</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 25</td>"\
            "<td>BR * 2 + 4</td>"\
            "<td>BR * 4 + 8</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Colony</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 25</td>"\
            "<td>BR * 2 + 4</td>"\
            "<td>BR * 4 + 8</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Stargate</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 100</td>"\
            "<td>BR * 2 + 16</td>"\
            "<td>BR * 4 + 32</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Cloaker</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 25</td>"\
            "<td>BR * 2 + 4</td>"\
            "<td>BR * 4 + 8</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Satellite</b></td>"\
            "<td>((BR + 4 ) ^ 2) - 10</td>"\
            "<td>BR * 2 - 2&nbsp; (*)</td>"\
            "<td>0</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Terraformer</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 25</td>"\
            "<td>BR * 2 + 4</td>"\
            "<td>BR * 4 + 8</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Troopship</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 25</td>"\
            "<td>BR * 2 + 4</td>"\
            "<td>BR * 4 + 8</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Doomsday</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 10</td>"\
            "<td>BR * 2 + 2</td>"\
            "<td>BR * 4 + 4</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Minefield</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 10</td>"\
            "<td>BR * 2 + 2</td>"\
            "<td>0</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Minesweeper</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 25</td>"\
            "<td>BR * 2 + 4</td>"\
            "<td>BR * 4 + 8</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Engineer</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 100</td>"\
            "<td>BR * 2 + 16</td>"\
            "<td>BR * 4 + 32</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Carrier</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 75</td>"\
            "<td>BR * 2 + 12</td>"\
            "<td>BR * 4 + 24</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Builder</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 50</td>"\
            "<td>BR * 2 + 8</td>"\
            "<td>BR * 4 + 16</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Morpher</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 35</td>"\
            "<td>BR * 2 + 6</td>"\
            "<td>BR * 4 + 12</td>"\
        "</tr>"\
        "<tr>"\
            "<td><b>Jumpgate</b></td>"\
            "<td>((BR + 4 ) ^ 2) + 150</td>"\
            "<td>BR * 2 + 24</td>"\
            "<td>BR * 4 + 48</td>"\
        "</tr>"\
        "<tr>"\
            "<td colspan=\"4\" align=\"center\">(*) If this value falls below 2, it becomes 2</td>"\
        "</tr>"\
        "</table></center>"
    );

    // Planets and maps
    OutputText ("</strong></li></ul><p><center><h2>Planets and maps</h2></center>"\
        "<ul><li>Planets that have been nuked <strong>");
    m_pHttpResponse->WriteText (gcConfig.iNukesForQuarantine);
    OutputText ("</strong> or more times are automatically quarantined for <strong>");
    
    m_pHttpResponse->WriteText (gcConfig.iUpdatesInQuarantine);
    OutputText ("</strong> updates after each nuke and have their agriculture reduced to zero</li>"\
        "<li>When generating a map, the chance that a planet will have a link to an adjacent "\
        "planet that it wasn't originally linked to is <strong>");
    
    m_pHttpResponse->WriteText (mcConfig.iChanceNewLinkForms);
    OutputText ("%</strong></li><li>Planets can have up to <strong>");
    m_pHttpResponse->WriteText (mcConfig.fResourceAllocationRandomizationFactor);
    OutputText ("</strong> times their average allocation in resources</li>"\
        
        "<li>The maximum map deviation is <strong>");
    m_pHttpResponse->WriteText (mcConfig.iMapDeviation);
    OutputText ("</strong>. This number determines the approximate coordinates of the first planet on the map</li>"
        
        "<li>The chance that a new planet in a chain will be linked to the last created planet "\
        "in that chain is <strong>");
    m_pHttpResponse->WriteText (mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap);
    OutputText ("%</strong> for large maps and <strong>");
    m_pHttpResponse->WriteText (mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap);
    OutputText ("%</strong> for small maps</li>"\
        
        "<li>Large maps are those with <strong>");
    m_pHttpResponse->WriteText (mcConfig.iLargeMapThreshold);
    OutputText ("</strong> or more planets per empire</li>"\
        
        "</ul>"\
        "<p><center><h2>Gameplay</h2></center>"\
        "<ul><li>Empires can quit from a game at any time before the game starts</li>"\
        "<li>When an empire enters or quits from a game, its name is broadcast to everyone else in the game</li>"\
        "<li>The IP addresses that appear in the diplomacy and profile pages are not real addresses.  However, "\
        "two empires played from the same IP address will have the same value.  But beware of accusing "\
        "players of multi-emping when they're simply behind the same firewall, as some "\
        "ISPs such as WebTV use proxy servers for web traffic, which causes their users "\
        "to appear to be coming from the same IP address.</li>"\
        "<li>The tech level of empires who join a game late is determined by adding to the initial BR for "\
        "the game <strong>");
    
    m_pHttpResponse->WriteText (gcConfig.iPercentTechIncreaseForLatecomers);
    OutputText ("%</strong> of the product of the number of updates missed "\
        "times the max increase allowed by the gameclass</li>"\
        "<li>The first trade agreement will increase an empire's econ by <strong>");
    
    m_pHttpResponse->WriteText (gcConfig.iPercentFirstTradeIncrease);
    OutputText ("%</strong>. Subsequent trade agreements will give <strong>");
    m_pHttpResponse->WriteText (gcConfig.iPercentNextTradeIncrease);
    OutputText ("%</strong> of the previous increase</li>");
    
    OutputText ("<li><strong>");
    m_pHttpResponse->WriteText (gcConfig.iPercentDamageUsedToDestroy);
    OutputText (
        "%</strong> of TOT_DMG is converted to DEST during ship combat</li>"\
        "<li>If all empires in a game are idle for <strong>"
        );

    m_pHttpResponse->WriteText (NUM_UPDATES_FOR_GAME_RUIN);
    
    OutputText (
        "</strong> updates, then the game will end automatically</li>"\
        "<li>If the server is down for <strong>"
        );
    
    m_pHttpResponse->WriteText (vNumUpdatesDown.GetInteger());
    OutputText ("</strong> update");
    if (vNumUpdatesDown.GetInteger() != 1) {
        OutputText ("s");
    } 
    OutputText (" of a short term game, then the game automatically ends</li>"\
        "<li>Short term games are those with update periods of less than <strong>");
    
    WriteTime (vSecondsForLongtermStatus.GetInteger());
    
    OutputText (
        "</strong></li>"\
        "<li>Games with no weekend updates postpone updates scheduled to occur on a weekend until the following Monday"\
        " at 00:00:00 server time plus ");
    
    WriteTime (vAfterWeekendDelay.GetInteger());
    
    OutputText ("</li><li>Grudge games <strong>");
    
    if (!(iSystemOptions & DEFAULT_BRIDIER_GAMES)) {
        OutputText ("do not ");
    }
    
    OutputText (
        "count</strong> towards Bridier Scoring by default</li>"\
        
        "<li>The default IP Address filtering is "
        );
    
    if ((iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) ==
        (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) {
        OutputText ("<strong>warn</strong> and <strong>reject</strong>");
    }
    
    else if (iSystemOptions & DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
        OutputText ("<strong>warn</strong>");
    }
    
    else if (iSystemOptions & DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
        OutputText ("<strong>reject</strong>");
    }
    
    else {
        OutputText ("<strong>none</strong>");
    }
    OutputText ("</li>"
        
        "<li>The default Session Id filtering is "
        );
    
    if ((iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) ==
        (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) {
        OutputText ("<strong>warn</strong> and <strong>reject</strong>");
    }
    
    else if (iSystemOptions & DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
        OutputText ("<strong>warn</strong>");
    }
    
    else if (iSystemOptions & DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
        OutputText ("<strong>reject</strong>");
    }
    
    else {
        OutputText ("<strong>none</strong>");
    }
    
    OutputText ("</li><li>The default number of updates before a game closes is <strong>");
    m_pHttpResponse->WriteText (vDefaultNumUpdatesForClose.GetInteger());
    
    OutputText ("</strong>"\
        
        "</li></ul><p><center><h2>User Interface</h2></center><ul>"\
        
        "<li>Empire names are case insensitive and all beginning and trailing spaces are automatically removed, as are intermediate double spaces</li>"\
        "<li>Empire names and user input are fully filtered for HTML content</li>"\
        "<li>The chatroom will allow a maximum of <strong>");
    
    m_pHttpResponse->WriteText(global.GetChatroom()->GetMaxNumSpeakers());
    OutputText ("</strong> simultaneous empires</li>");
    
    OutputText ("<li>The chatroom will display the last <strong>");
    m_pHttpResponse->WriteText(global.GetChatroom()->GetMaxNumMessages());
    OutputText ("</strong> messages</li>");
    
    OutputText ("<li>Empires time out of the chatroom when they are idle for ");
    WriteTime(global.GetChatroom()->GetTimeOut());
    
    OutputText ("</li><li>The maximum size of an uploaded alien icon is <strong>");
    iErrCode = GetSystemProperty(SystemData::MaxIconSize, &vValue);
    RETURN_ON_ERROR(iErrCode);
    m_pHttpResponse->WriteText(vValue.GetInteger());
    OutputText ("</strong> bytes</li>");

    OutputText ("<li>The default icon for new empires is: ");
    Variant vAlienKey, vAlienAddress;
    iErrCode = GetSystemProperty(SystemData::DefaultAlienKey, &vAlienKey);
    RETURN_ON_ERROR(iErrCode);
    iErrCode = GetSystemProperty(SystemData::DefaultAlienAddress, &vAlienAddress);
    RETURN_ON_ERROR(iErrCode);
    iErrCode = WriteEmpireIcon(vAlienKey, vAlienAddress, NO_KEY, NULL, false);
    RETURN_ON_ERROR(iErrCode);
    OutputText ("</li>");

    OutputText ("<li>The icon used for system messages is: ");
    iErrCode = GetSystemProperty(SystemData::SystemMessagesAlienKey, &vAlienKey);
    RETURN_ON_ERROR(iErrCode);
    iErrCode = GetSystemProperty(SystemData::SystemMessagesAlienAddress, &vAlienAddress);
    RETURN_ON_ERROR(iErrCode);
    iErrCode = WriteEmpireIcon(vAlienKey, vAlienAddress, NO_KEY, NULL, false);
    RETURN_ON_ERROR(iErrCode);
    OutputText ("</li>");
    
    OutputText ("</ul><center><h2>Score</h2></center><ul><li>A win gives <strong>");
    
    m_pHttpResponse->WriteText (CLASSIC_POINTS_FOR_WIN);
    OutputText ("</strong> classic points</li><li>A draw gives <strong>");
    
    m_pHttpResponse->WriteText (CLASSIC_POINTS_FOR_DRAW);
    OutputText ("</strong> classic points</li><li>A nuke gives <strong>");
    
    m_pHttpResponse->WriteText (CLASSIC_POINTS_FOR_NUKE);
    OutputText ("</strong> classic points</li><li>Being nuked subtracts <strong>");

    m_pHttpResponse->WriteText (- CLASSIC_POINTS_FOR_NUKED);
    OutputText ("</strong> classic points</li><li>Ruining subtracts <strong>");
    
    m_pHttpResponse->WriteText (- CLASSIC_POINTS_FOR_RUIN);
    OutputText ("</strong> classic points</li><li>The Almonaster base unit is <strong>");
    
    m_pHttpResponse->WriteText (ALMONASTER_BASE_UNIT);
    OutputText ("</strong> points</li><li>The minimum Almonaster score is <strong>");
    
    m_pHttpResponse->WriteText (ALMONASTER_MIN_SCORE);

    OutputText ("</strong> points</li><li>The initial Almonaster score is <strong>");
    m_pHttpResponse->WriteText (ALMONASTER_INITIAL_SCORE);
    
    OutputText ("</strong> points</li><li>The Almonaster score increase factor is between <strong>");
    m_pHttpResponse->WriteText (ALMONASTER_MAX_INCREASE_FACTOR);
    OutputText ("</strong> and <strong>");
    m_pHttpResponse->WriteText (ALMONASTER_MAX_DECREASE_FACTOR);
    
    OutputText ("</strong></li><li>The Almonaster score alliance ratio is between <strong>");
    m_pHttpResponse->WriteText (ALMONASTER_MIN_ALLIANCE_RATIO);
    OutputText ("</strong> and <strong>");
    m_pHttpResponse->WriteText (ALMONASTER_MAX_ALLIANCE_RATIO);
    
    OutputText ("</strong></li><li>The Almonaster score nuker significance ratio is between <strong>");
    m_pHttpResponse->WriteText (ALMONASTER_MIN_NUKER_SIGNIFICANCE_RATIO);
    OutputText ("</strong> and <strong>");
    m_pHttpResponse->WriteText (ALMONASTER_MAX_NUKER_SIGNIFICANCE_RATIO);
    
    OutputText ("</strong></li><li>The Almonaster score nuked significance ratio is between <strong>");
    m_pHttpResponse->WriteText (ALMONASTER_MIN_NUKED_SIGNIFICANCE_RATIO);
    OutputText ("</strong> and <strong>");
    m_pHttpResponse->WriteText (ALMONASTER_MAX_NUKED_SIGNIFICANCE_RATIO);
    OutputText ("</strong></li>");

    if (iSystemOptions & DISABLE_PRIVILEGE_SCORE_ELEVATION) {

        OutputText ("<li>Almonaster scores do not give special privileges to any empires."\
            " Only an administrator can assign apprentice and adept privileges.</li>");

    } else {
    
        iErrCode = GetScoreForPrivilege (PRIVILEGE_FOR_PERSONAL_GAMES, &fValue);
        RETURN_ON_ERROR(iErrCode);
            
        OutputText ("<li>Empires with Almonaster scores greater than <strong>");
        m_pHttpResponse->WriteText (fValue);    
        OutputText ("</strong> are considered <strong>");
        m_pHttpResponse->WriteText (PRIVILEGE_STRING_PLURAL [PRIVILEGE_FOR_PERSONAL_GAMES]);
        OutputText ("</strong> and have the right to create their own personal games</li>");
        
        iErrCode = GetScoreForPrivilege(PRIVILEGE_FOR_PERSONAL_GAMECLASSES, &fValue);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetSystemProperty(SystemData::MaxNumPersonalGameClasses, &vValue);
        RETURN_ON_ERROR(iErrCode);

        OutputText ("<li>Empires with Almonaster scores greater than <strong>");
        m_pHttpResponse->WriteText (fValue);
        OutputText ("</strong> are considered <strong>");
        m_pHttpResponse->WriteText (PRIVILEGE_STRING_PLURAL [PRIVILEGE_FOR_PERSONAL_GAMECLASSES]);
        OutputText ("</strong> and have the right to create personal Tournaments and up to <strong>");
        m_pHttpResponse->WriteText (vValue.GetInteger());
        OutputText ("</strong> personal GameClasses</li>");
    }

    iErrCode = GetSystemProperty (SystemData::PrivilegeForUnlimitedEmpires, &vUnlimitedEmpirePrivilege);
    RETURN_ON_ERROR(iErrCode);

    OutputText ("<li>Empires with a privilege level of <strong>");
    m_pHttpResponse->WriteText (PRIVILEGE_STRING [vUnlimitedEmpirePrivilege.GetInteger()]);
    OutputText ("</strong> or greater have the right to create games with an unlimited maximum number of empires");

    OutputText (
        "</li>"\
        "<li>Empires with no Bridier activity for three months will see their Bridier Index "\
        "set to <strong>200</strong> if it is less.</li>"\
        "<li>Empires with no Bridier activity for four months will see their Bridier Index "\
        "set to <strong>300</strong> if it is less.</li>"\
        "<li>Empires with no Bridier activity for five months will see their Bridier Index "\
        "set to <strong>400</strong> if it is less.</li>"\
        "<li>Empires with no Bridier activity for six months will see their Bridier Index "\
        "set to <strong>500</strong> if it is less.</li>"\
        "<li>Empires will be checked for Bridier activity every "
        );

    WriteTime (sBridierScanFrequency);
    OutputText (
        "</li>"\
        "<li>Up to <strong>"
        );
    
    m_pHttpResponse->WriteText (vNumNukesListed.GetInteger());
    OutputText ("</strong> nuke");
    if (vNumNukesListed.GetInteger() != 1) {
        OutputText ("s");
    }
    
    OutputText (
        " will be listed in each empire's nuke history</li>"\
        "</ul><center>"
        );
    
/*
        "<h2>Nomenclature</h2></center>"
        
        "<center><table width=\"75%\" border=\"1\">"\
        "<tr>"\
        "<td><strong>Weekend Updates (Weekend)</strong></td>"\
        "<td>The game updates on weekends</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Private Messages (PrivateMsg)</strong></td>"\
        "<td>Private messages can be sent between empires</td>"\
        "</tr>"\

        "<tr>"\
        "<td><strong>Exposed Diplomacy (DipExposed)</strong></td>"\
        "<td>All empires in the game have met each other initially</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Exposed Maps (MapExposed)</strong></td>"\
        "<td>All planets are visible to all empires initially</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Shared Maps (MapShared)</strong></td>"\
        "<td>Empires with a given diplomatic status share their maps</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Fully Colonized Maps</strong></td>"\
        "<td>Empires begin the game with their planets already colonized</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Disconnected Maps (Disconnected)</strong></td>"\
        "<td>Empires' planets are not connected to other empires' planets when the game begins</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Visible Builds (VisibleBuilds)</strong></td>"\
        "<td>Ships are visible to other empires during the update in which they are built</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Visible Diplomacy (VisibleDip)</strong></td>"\
        "<td>Diplomacy offerings are visible during the update in which they are offered</td>"\
        "</tr>"\

        "<tr>"\
        "<td><strong>Population needed to Build (BuildPop)</strong></td>"\
        "<td>The population needed at a planet for it to be able to build ships</td>"\
        "</tr>"\

        "<tr>"\
        "<td><strong>Maximum number of ships allowed (MaxShips)</strong></td>"\
        "<td>The maximum number of ships an empire can have in operation at any given time</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Maximum Ag Ratio (MaxAgRatio)</strong></td>"\
        "<td>The maximum possible ag ratio for an empire during the game</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Updates for Idle Status (IdleUpdates)</strong></td>"\
        "<td>The number of updates an empire can be absent from the game before it becomes idle "\
        "and is automatically ready for an update</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Fair diplomacy</strong></td>"\
        "<td>Empires are limited to a maximum of (N - 2) / 2 empires at the specified diplomacy level,"\
        "where N is the greatest number of empires in the game at any one time"\
        "</td></tr>"\
        
        "<tr>"\
        "<td><strong>X truces, trades or alliances</strong></td>"\
        "<td>Empires are limited to a maximum of X empires at a certain level of diplomacy. "\
        "Alliances count as trades and truces, and trades count as truces. If alliance limits are set to "\
        "count for the entire game, then any alliance that ceases to exist (due to an annihilation, or at "\
        "least one empire dropping down to another level), then that alliance will count for the entire game "\
        "and will limit the ability of the affected empires to establish other alliances. "\
        "Empires who were previously allied can ally again with no additional counts added."\
        "</td></tr>"\
        
        "<tr>"\
        "<td><strong>Unbreakable alliances</strong></td>"\
        "<td>If alliances are unbreakable, then empires who are at alliance cannot drop down to any "\
        "other diplomatic level and will remain at alliance until one of them leaves the game."\
        "</td></tr>"\
        
        "<tr>"\
        "<td><strong>Surrenders</strong></td>"\
        "<td>Surrenders can only take place between two empires who are currently at war and who have "\
        "never been allied in the course of the game."\
        "</td></tr>"\
        
        "<tr>"\
        "<td><strong>Classic SC-style Surrenders (Classic)</strong></td>"\
        "<td>Surrendering empires leave the game immediately, but when their homeworlds are colonized, "\
        "the colonizers are considered to have nuked the empire"\
        "</td></tr>"\
        
        "<tr>"\
        "<td><strong>Permanent Doomsdays (PermanentDooms)</strong></td>"\
        "<td>Doomsdays behave as in classic Stellar Crisis: annihilated planets remain in quarantine forever,"\
        " and never revert to colonizable status."\
        "</td></tr>"\

        "<tr>"\
        "<td><strong>Friendly Doomsdays (FriendlyDooms)</strong></td>"\
        "<td>Friendly doomsdays cannot annihilate planets belonging to non-warring empires."\
        "</td></tr>"\

        "<tr>"\
        "<td><strong>Suicidal Doomsdays (SuicidalDooms)</strong></td>"\
        "<td>Doomsdays can annihilate planets belonging to their owner, except for their own homeworld."\
        "</td></tr>"\

        "<tr>"\
        "<td><strong>Friendly Gates (FriendlyGates)</strong></td>"\
        "<td>Stargates and jumpgates can gate ships belonging to allied empires"\
        "</td></tr>"\

        "<tr>"\
        "<td><strong>Friendly Sciences (FriendlyScis)</strong></td>"\
        "<td>Science ships cannot nuke enemy planets"\
        "</td></tr>"\

        "<tr>"\
        "<td><strong>Independence (Independence)</strong></td>"\
        "<td>When an empire is nuked, its territories remain alive and populated as independent planets "\
        "that cannot simply be re-colonized by other empires. Similarly, the empire's ships remain alive, and "\
        "either remain independent or revert to belonging to the owner of the planet at which they were located</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Subjective Views (Subjective)</strong></td>"\
        "<td>The empire econ and mil totals displayed in the diplomacy screen for each empire represent the "\
        "portion of the empire's resources that can be seen by the viewing empire on his or her map."\
        "</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Simple Ruins (SimpleRuins)</strong></td>"\
        "<td>Games with simple ruins enabled will behave just like classic Stellar Crisis: if an empire "\
        "has been idle for a gameclass-specific number of updates, then it will fall into ruin and be "\
        "removed from the game"\
        "</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td><strong>Complex Ruins (ComplexRuins)</strong></td>"\
        "<td>Games with complex ruins enabled function as follows: if all empires or all empires except one "\
        "are idle for a gameclass-specific number of updates, then the idle empires will fall into ruin, but "\
        "only if more than two empires were in the game at some point during its lifetime."\
        "</td>"\
        "</tr>"\
        
        "</table>"
*/

    return iErrCode;
}