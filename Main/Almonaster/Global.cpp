//
// Almonaster.dll:  a component of Almonaster
// Copyright(c) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include "Osal/File.h"

#include "Global.h"
#include "GameEngineSchema.h"

#include "AlmonasterScore.h"
#include "ClassicScore.h"
#include "BridierScore.h"
#include "TournamentScoring.h"

//
// TLS connection management
//

// Yes, not Linux-friendly. Sorry.
__declspec(thread) IDatabaseConnection* t_pConn = NULL;
__declspec(thread) ICachedTableCollection* t_pCache = NULL;
__declspec(thread) Uuid t_uuidReq;

Global global;

Variant* AutoFreeData::g_pvData = NULL;
Variant** AutoFreeData::g_ppvData = NULL;

Global::Global() 
    : m_pHttpServer(NULL),
      m_pConfig(NULL),
      m_pPageSourceControl(NULL),
      m_pFileCache(NULL),
      m_pDatabase(NULL),
      m_iRootKey(NO_KEY),
      m_iGuestKey(NO_KEY)
{
    m_pszResourceDir[0] = '\0';
}

void Global::Close()
{
    // Weak refs
    //SafeRelease(m_pHttpServer);
    //SafeRelease(m_pPageSourceControl);
    
    m_asyncManager.Close();

    // Strong refs
    SafeRelease(m_pFileCache);
    SafeRelease(m_pConfig);

    SafeRelease(m_pDatabase);
    m_libDatabase.Close();
}

void Global::TlsOpenConnection()
{
    t_pConn = m_pDatabase->CreateConnection(SERIALIZABLE);
    Assert(t_pConn);

    t_pCache = t_pConn->GetCache();
    Assert(t_pCache);
}

int Global::TlsCommitTransaction()
{
    return t_pConn->Commit();
}

void Global::TlsCloseConnection()
{
    t_pCache = NULL; // No reference held
    SafeRelease(t_pConn);
}

void Global::InitRequestId()
{
    int iErrCode = OS::CreateUuid(&t_uuidReq);
    Assert(iErrCode == OK);
}

void Global::GetRequestId(Uuid* puuidReqId)
{
    *puuidReqId = t_uuidReq;
}

void TraceError(int iErrCode, const char* pszFile, int iLine)
{
    Uuid uuidReqId;
    global.GetRequestId(&uuidReqId);

    char pszUuidReqId[OS::MaxUuidLength];
    int err = OS::StringFromUuid(uuidReqId, pszUuidReqId);
    Assert(err == OK);

    Thread thread;
    Thread::GetCurrentThread(&thread);

    char pszError[512];
    sprintf(pszError, "Thread %d - Request %s - Error %i occurred in %s line %i", thread.GetThreadId(), pszUuidReqId, iErrCode, pszFile, iLine);
    global.WriteReport(TRACE_ERROR, pszError);
}

int Global::Initialize(IHttpServer* pHttpServer, IPageSourceControl* pPageSourceControl)
{
    // Weak refs
    m_pHttpServer = pHttpServer;
    m_pPageSourceControl = pPageSourceControl;

    // Strong refs
    m_pFileCache = pHttpServer->GetFileCache();
    m_pConfig = pPageSourceControl->GetConfigFile();

    int iErrCode;
    char* pszTemp;

    //
    // Configuration
    //

    ITraceLog* pReport = GetReport();
    AutoRelease<ITraceLog> release_pReport(pReport);

    // DatabaseLibrary
    iErrCode = m_pConfig->GetParameter ("DatabaseLibrary", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL)
    {
        pReport->Write(TRACE_ERROR, "Error: Could not read the DatabaseLibrary value from the configuration file");
        return ERROR_FAILURE;
    }
    char pszDatabaseLibrary[OS::MaxFileNameLength];
    if (File::ResolvePath(pszTemp, pszDatabaseLibrary) == ERROR_FAILURE)
    {
        pReport->Write(TRACE_ERROR, "Error: The DatabaseLibrary value from the configuration file was invalid");
        return ERROR_FAILURE;
    }

    // DatabaseClsid
    iErrCode = m_pConfig->GetParameter("DatabaseClsid", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL)
    {
        pReport->Write(TRACE_ERROR, "Error: Could not read the DatabaseClsid value from the configuration file");
        return ERROR_FAILURE;
    }
    Uuid uuidDatabaseClsid;
    iErrCode = OS::UuidFromString(pszTemp, &uuidDatabaseClsid);
    if (iErrCode != OK)
    {
        pReport->Write(TRACE_ERROR, "Error: The DatabaseClsid value from the configuration file was invalid");
        return ERROR_FAILURE;
    }

    // DatabaseConnectionString
    char* pszDatabaseConnectionString;
    iErrCode = m_pConfig->GetParameter("DatabaseConnectionString", &pszDatabaseConnectionString);
    if (iErrCode != OK || pszDatabaseConnectionString == NULL)
    {
        pReport->Write(TRACE_ERROR, "Error: Could not read the DatabaseConnectionString value from the configuration file");
        return ERROR_FAILURE;
    }

    // Resource directory
    iErrCode = m_pConfig->GetParameter ("ResourceDirectory", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL)
    {
        pReport->Write(TRACE_ERROR, "Error: Could not read the ResourceDirectory value from the configuration file");
        return ERROR_FAILURE;
    }
    if (File::ResolvePath(pszTemp, m_pszResourceDir) != OK)
    {
        pReport->Write(TRACE_ERROR, "Error: The ResourceDirectory value from the configuration file was invalid");
        return ERROR_FAILURE;
    }

    iErrCode = InitializeDatabase(pszDatabaseLibrary, uuidDatabaseClsid, pszDatabaseConnectionString);
    if (iErrCode != OK && iErrCode != WARNING)
    {
        pReport->Write(TRACE_ERROR, "Failed to initialize database");
        return iErrCode;
    }

    TlsOpenConnection();

    GameEngine gameEngine;
    iErrCode = gameEngine.Setup();
    RETURN_ON_ERROR(iErrCode);

    iErrCode = InitializeState();
    RETURN_ON_ERROR(iErrCode);

    iErrCode = InitializeChatroom();
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = TlsCommitTransaction();
    RETURN_ON_ERROR(iErrCode);

    TlsCloseConnection();

    // Initialize AsyncManager
    iErrCode = m_asyncManager.Initialize();
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int Global::InitializeState()
{
    int iErrCode;
    GameEngine gameEngine;
    
    iErrCode = gameEngine.LookupEmpireByName(ROOT_NAME, &m_iRootKey, NULL, NULL);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = gameEngine.LookupEmpireByName(GUEST_NAME, &m_iGuestKey, NULL, NULL);
    RETURN_ON_ERROR(iErrCode);

    if (m_iRootKey == NO_KEY || m_iGuestKey == NO_KEY)
    {
        return ERROR_FAILURE;
    }
    return iErrCode;
}

int Global::InitializeChatroom()
{
    int iErrCode;
    char* pszTemp;

    ChatroomConfig ccConfig;
    ccConfig.cchMaxSpeakerNameLen = MAX_EMPIRE_NAME_LENGTH;

    //
    // Config
    //

    ITraceLog* pReport = GetReport();
    AutoRelease<ITraceLog> release_pReport(pReport);

    iErrCode = m_pConfig->GetParameter ("ChatroomMaxNumSpeakers", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL)
    {
        pReport->Write(TRACE_ERROR, "Error: Could not read the ChatroomMaxNumSpeakers value from the configuration file");
        return ERROR_FAILURE;
    }
    ccConfig.iMaxNumSpeakers = atoi (pszTemp);

    iErrCode = m_pConfig->GetParameter ("ChatroomNumMessages", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL)
    {
        pReport->Write(TRACE_ERROR, "Error: Could not read the ChatroomNumMessages value from the configuration file");
        return ERROR_FAILURE;
    }
    ccConfig.iMaxNumMessages = atoi (pszTemp);

    iErrCode = m_pConfig->GetParameter ("ChatroomMaxMessageLength", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL)
    {
        pReport->Write(TRACE_ERROR, "Error: Could not read the ChatroomMaxMessageLength value from the configuration file");
        return ERROR_FAILURE;
    }
    ccConfig.iMaxMessageLength = atoi (pszTemp);

    iErrCode = m_pConfig->GetParameter ("ChatroomTimeOut", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL)
    {
        pReport->Write(TRACE_ERROR, "Error: Could not read the ChatroomTimeOut value from the configuration file");
        return ERROR_FAILURE;
    }
    ccConfig.sTimeOut = atoi(pszTemp);
    if (ccConfig.sTimeOut == 0)
    {
        pReport->Write(TRACE_ERROR, "Error: Invalid ChatroomTimeOut value from the configuration file");
        return ERROR_FAILURE;
    }

    iErrCode = m_pConfig->GetParameter ("ChatroomPostSystemMessages", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL)
    {
        pReport->Write(TRACE_ERROR, "Error: Could not read the ChatroomPostSystemMessages value from the configuration file");
        return ERROR_FAILURE;
    }
    ccConfig.bPostSystemMessages = atoi(pszTemp) != 0;

    iErrCode = m_cChatroom.Initialize(ccConfig);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int Global::InitializeDatabase(const char* pszLibDatabase, const Uuid& uuidDatabaseClsid, const char* pszDatabaseConnectionString)
{
    ITraceLog* pReport = GetReport();
    AutoRelease<ITraceLog> release_pReport(pReport);

    pReport->Write(TRACE_ALWAYS, "Loading the database library");

    int iErrCode = m_libDatabase.Open(pszLibDatabase);
    if (iErrCode != OK)
    {
        pReport->Write(TRACE_ERROR, "Unable to load database library");
        return iErrCode;
    }

    typedef int(*Fxn_CreateInstance)(const Uuid&, const Uuid&, void**);
    Fxn_CreateInstance pCreateInstance = (Fxn_CreateInstance)m_libDatabase.GetExport("CreateInstance");
    if (pCreateInstance == NULL)
    {
        pReport->Write(TRACE_ERROR, "Unable to obtain CreateInstance export from database library");
        return ERROR_FAILURE;
    }

    iErrCode = pCreateInstance(uuidDatabaseClsid, IID_IDatabase, (void**)&m_pDatabase);
    if (iErrCode != OK)
    {
        pReport->Write(TRACE_ERROR, "Unable to create database instance");
        return iErrCode;
    }

    pReport->Write(TRACE_ALWAYS, "Loaded the database library");
    pReport->Write(TRACE_ALWAYS, "Initializing the database");

    for (int i = 0; i < 30; i ++)
    {
        iErrCode = m_pDatabase->Initialize(pszDatabaseConnectionString, pReport);
        if (iErrCode == OK)
        {
            pReport->Write(TRACE_ALWAYS, "Reloaded database sucessfully");
            break;
        }
        else if (iErrCode == WARNING)
        {
            pReport->Write(TRACE_ALWAYS, "New database successfully created");
            break;
        }
        else if (iErrCode == ERROR_DATABASE_EXCEPTION)
        {
            // We may be booting, and SQL Server may not be accepting connections yet
            // Wait a second and keep trying; maybe we'll get lucky...
            pReport->Write(TRACE_ALWAYS, "Initializing database failed with ERROR_DATABASE_EXCEPTION");
            OS::Sleep(1000);
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (iErrCode != OK && iErrCode != WARNING)
    {
        TRACE_ERROR(iErrCode);
        pReport->Write(TRACE_ERROR, "Timed out initializing database after 10 retries");
    }
    return iErrCode;
}

void Global::WriteReport(TraceInfoLevel level, const char* pszMessage)
{
    ITraceLog* pReport = GetReport();
    AutoRelease<ITraceLog> release_pReport(pReport);

    pReport->Write(level, pszMessage);
}