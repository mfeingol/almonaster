//
// Almonaster.dll:  a component of Almonaster
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

#include "Almonaster.h"

#include "GameEngine/GameEngine.h"
#include "HtmlRenderer/HtmlRenderer.h"
#include "Chatroom/CChatroom.h"

#include "Osal/File.h"
#include "Osal/Vector.h"

///////////////////////////////
// PageSource implementation //
///////////////////////////////

const Uuid CLSID_Almonaster = { 0x8b631302, 0x8cfa, 0x11d3, { 0xa2, 0x40, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };

const Uuid IID_IGameEngine = { 0xb7365051, 0xbf4f, 0x11d3, { 0xa2, 0xb6, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IAlmonasterHook = { 0xb7365052, 0xbf4f, 0x11d3, { 0xa2, 0xb6, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IAlmonasterUIEventSink = { 0xb7365053, 0xbf4f, 0x11d3, { 0xa2, 0xb6, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IMapGenerator = { 0x2751ca22, 0x493a, 0x4bdb, { 0x85, 0xcd, 0x7b, 0x59, 0xa2, 0xf8, 0x88, 0x79 } };
const Uuid IID_IScoringSystem = { 0xde61fe7e, 0xb8e8, 0x4bcd, { 0x96, 0xeb, 0xdc, 0xf7, 0x96, 0x52, 0xe, 0xb } };

//
// Page source class
//
class Almonaster : public IPageSource, public IAlmonasterUIEventSink {
private:

    Almonaster();
    ~Almonaster();

    bool m_bIsDefault;
    bool m_bNoBuffering;

    char* m_pszUri1;
    char* m_pszUri2;

    int OnAccessDenied (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);

public:

    static Almonaster* CreateInstance();

    IMPLEMENT_INTERFACE (IPageSource);

    // IPageSource
    int OnInitialize (IHttpServer* pHttpServer, IPageSourceControl* pControl);
    int OnFinalize();

    int OnGet (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);
    int OnPost (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);

    int OnBasicAuthenticate (const char* pszLogin, const char* pszPassword, bool* pbAuthenticate);

    int OnError (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);

    // IAlmonasterUIEventSink
    int OnLoginEmpire (int iEmpireKey);
    int OnCreateEmpire (int iEmpireKey);
    int OnDeleteEmpire (int iEmpireKey);
    int OnCreateGame (int iGameClass, int iGameNumber);
    int OnCleanupGame (int iGameClass, int iGameNumber);

    int OnDeleteTournament (unsigned int iTournamentKey);
    int OnDeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey);
};


//
// Implementation
//

Almonaster* Almonaster::CreateInstance() {
    return new Almonaster();
}

Almonaster::Almonaster() {

    m_bIsDefault = false;
    m_bNoBuffering = false;

    m_iNumRefs = 1;

    m_pszUri1 = NULL;
    m_pszUri2 = NULL;

    memset (&HtmlRenderer::m_sStats, 0, sizeof (AlmonasterStatistics));
}

Almonaster::~Almonaster() {

    if (!m_bIsDefault && m_pszUri1 != NULL) {
        delete [] m_pszUri1;
    }
}

extern "C" EXPORT int CreateInstance (const Uuid& uuidClsid, const Uuid& uuidIid, void** ppObject) {
    
    if (uuidClsid == CLSID_Almonaster) {
        
        if (uuidIid == IID_IPageSource) {
            *ppObject = (void*) static_cast<IPageSource*> (Almonaster::CreateInstance());
            return *ppObject == NULL ? ERROR_OUT_OF_MEMORY : OK;
        }

        if (uuidIid == IID_IObject) {
            *ppObject = (void*) static_cast<IObject*> (Almonaster::CreateInstance());
            return *ppObject == NULL ? ERROR_OUT_OF_MEMORY : OK;
        }
        
        return ERROR_NO_INTERFACE;
    }
    
    return ERROR_NO_CLASS;
}

// System globals
IHttpServer* g_pHttpServer = NULL;
IReport* g_pReport = NULL;
IConfigFile* g_pConfig = NULL;
ILog* g_pLog = NULL;
IPageSourceControl* g_pPageSourceControl = NULL;
IFileCache* g_pFileCache = NULL;

// Game objects
GameEngine* g_pGameEngine = NULL;
Chatroom* g_pChatroom = NULL;

char* g_pszResourceDir = NULL;

int Almonaster::OnInitialize (IHttpServer* pHttpServer, IPageSourceControl* pPageSourceControl) {

    int iMaxNumSpeakers, iMaxNumMessages, iMaxMessageLength, iErrCode;
    bool bPostSystemMessages;

    // Save weak refs
    g_pHttpServer = pHttpServer;
    g_pPageSourceControl = pPageSourceControl;

    g_pFileCache = pHttpServer->GetFileCache();
    g_pReport = g_pPageSourceControl->GetReport();
    g_pLog = g_pPageSourceControl->GetLog();
    g_pConfig = g_pPageSourceControl->GetConfigFile();

    // Prepare URI's
    m_bIsDefault = pPageSourceControl->IsDefault();

    if (m_bIsDefault) {

        m_pszUri1 = "/";
        m_pszUri2 = "";

    } else {

        const char* pszPageSourceName = g_pPageSourceControl->GetName();
        size_t stLen = strlen (pszPageSourceName) + 1;

        m_pszUri1 = new char [stLen * 2 + 3];

        if (m_pszUri1 == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        m_pszUri1[0] = '/';
        memcpy (m_pszUri1 + 1, pszPageSourceName, stLen);

        m_pszUri2 = m_pszUri1 + stLen + 1;
        memcpy (m_pszUri2, m_pszUri1, stLen);
        m_pszUri2[stLen] = '/';
        m_pszUri2[stLen + 1] = '\0';
    }

    g_pReport->WriteReport ("Reading parameters from configuration");

    // Read setup parameters
    Seconds sTimeOut;
    char* pszTemp = NULL;

    SystemConfiguration scConfig;

    const char* pszHookLibrary;
    char pszPath [OS::MaxFileNameLength];

    // Database Name
    iErrCode = g_pConfig->GetParameter ("DatabaseName", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the DatabaseName value from the configuration file");
        return ERROR_FAILURE;
    }

    char pszDatabaseName [OS::MaxFileNameLength];
    if (File::ResolvePath (pszTemp, pszDatabaseName) == ERROR_FAILURE) {
        g_pReport->WriteReport ("Error: The DatabaseName value from the configuration file was invalid");
        return ERROR_FAILURE;
    }
    size_t i, stLength = strlen (pszDatabaseName);
    for (i = 0; i < stLength; i ++) {
        if (pszDatabaseName[i] == '\\') {
            pszDatabaseName[i] = '/';
        }
    }

    // Resource directory
    iErrCode = g_pConfig->GetParameter ("ResourceDirectory", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the ResourceDirectory value from the configuration file");
        return ERROR_FAILURE;
    }

    char pszResourceDir [OS::MaxFileNameLength];
    if (File::ResolvePath (pszTemp, pszResourceDir) == ERROR_FAILURE) {
        g_pReport->WriteReport ("Error: The ResourceDirectory value from the configuration file was invalid");
        return ERROR_FAILURE;
    }
    stLength = strlen (pszResourceDir);
    for (i = 0; i < stLength; i ++) {
        if (pszResourceDir[i] == '\\') {
            pszResourceDir[i] = '/';
        }
    }

    iErrCode = g_pConfig->GetParameter ("ChatroomMaxNumSpeakers", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the ChatroomMaxNumSpeakers value from the configuration file");
        return ERROR_FAILURE;
    }
    iMaxNumSpeakers = atoi (pszTemp);

    iErrCode = g_pConfig->GetParameter ("ChatroomNumMessages", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the ChatroomNumMessages value from the configuration file");
        return ERROR_FAILURE;
    }
    iMaxNumMessages = atoi (pszTemp);

    iErrCode = g_pConfig->GetParameter ("ChatroomMaxMessageLength", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the ChatroomMaxMessageLength value from the configuration file");
        return ERROR_FAILURE;
    }
    iMaxMessageLength = atoi (pszTemp);

    iErrCode = g_pConfig->GetParameter ("ChatroomTimeOut", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the ChatroomTimeOut value from the configuration file");
        return ERROR_FAILURE;
    }
    sTimeOut = atoi (pszTemp);

    iErrCode = g_pConfig->GetParameter ("ChatroomPostSystemMessages", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the ChatroomPostSystemMessages value from the configuration file");
        return ERROR_FAILURE;
    }
    bPostSystemMessages = atoi (pszTemp) != 0;

    iErrCode = g_pConfig->GetParameter ("AutoBackup", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the AutoBackup value from the configuration file");
        return ERROR_FAILURE;
    }
    scConfig.bAutoBackup = atoi (pszTemp) != 0;

    iErrCode = g_pConfig->GetParameter ("NumHoursBetweenBackups", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the NumHoursBetweenBackups value from the configuration file");
        return ERROR_FAILURE;
    }
    scConfig.iSecondsBetweenBackups = atoi (pszTemp) * 60 * 60;

    iErrCode = g_pConfig->GetParameter ("BackupLifeTime", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the BackupLifeTime value from the configuration file");
        return ERROR_FAILURE;
    }
    scConfig.iBackupLifeTimeInSeconds = atoi (pszTemp) * 24 * 60 * 60;

    iErrCode = g_pConfig->GetParameter ("ReportEvents", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the ReportEvents value from the configuration file");
        return ERROR_FAILURE;
    }
    scConfig.bReport = atoi (pszTemp) != 0;

    iErrCode = g_pConfig->GetParameter ("HookLibrary", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the HookLibrary value from the configuration file");
        return ERROR_FAILURE;
    }
    if (pszTemp == NULL || *pszTemp == '\0') {
        pszHookLibrary = NULL;
    } else {

        if (File::ResolvePath (pszTemp, pszPath) == ERROR_FAILURE) {
            g_pReport->WriteReport ("Error: The HookLibrary value from the configuration file was invalid");
            return ERROR_FAILURE;
        }
        pszHookLibrary = pszPath;
    }

    iErrCode = g_pConfig->GetParameter ("BackupOnStartup", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the BackupOnStartup value from the configuration file");
        return ERROR_FAILURE;
    }
    scConfig.bBackupOnStartup = atoi (pszTemp) != 0;

    iErrCode = g_pConfig->GetParameter ("RebuildTopListsOnStartup", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the RebuildTopListsOnStartup value from the configuration file");
        return ERROR_FAILURE;
    }
    scConfig.bRebuildTopListsOnStartup = atoi (pszTemp) != 0;

    iErrCode = g_pConfig->GetParameter ("DatabaseCheck", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the DatabaseCheck value from the configuration file");
        return ERROR_FAILURE;
    }
    scConfig.bCheckDatabase = atoi (pszTemp) != 0;

    iErrCode = g_pConfig->GetParameter ("DatabaseWriteThrough", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the DatabaseWriteThrough value from the configuration file");
        return ERROR_FAILURE;
    }
    scConfig.bDatabaseWriteThrough = atoi (pszTemp) != 0;

    iErrCode = g_pConfig->GetParameter ("BufferedPageRendering", &pszTemp);
    if (iErrCode != OK || pszTemp == NULL) {
        g_pReport->WriteReport ("Error: Could not read the BufferedPageRendering value from the configuration file");
        return ERROR_FAILURE;
    }
    m_bNoBuffering = atoi (pszTemp) == 0;


    g_pReport->WriteReport ("Finished reading parameters from configuration file");

    // Sanity checks
    if (scConfig.bAutoBackup && scConfig.iSecondsBetweenBackups == 0) {
        g_pReport->WriteReport ("Error: The value for NumHoursBetweenBackups in the configuration file is illegal");
        return ERROR_FAILURE;
    }

    if (scConfig.bAutoBackup && scConfig.iBackupLifeTimeInSeconds == 0) {
        g_pReport->WriteReport ("Error: The value for BackupLifeTime in the configuration file is illegal");
        return ERROR_FAILURE;
    }

    if (scConfig.bAutoBackup && scConfig.iBackupLifeTimeInSeconds <= scConfig.iSecondsBetweenBackups) {
        g_pReport->WriteReport ("Error: The value for BackupLifeTime and NumHoursBetweenBackups in the configuration file is illegal");
        return ERROR_FAILURE;
    }

    if (iMaxNumSpeakers < 2) {
        g_pReport->WriteReport ("Error: The value for ChatroomMaxNumSpeakers in the configuration file is illegal");
        return ERROR_FAILURE;
    }

    if (iMaxNumMessages < 5) {
        g_pReport->WriteReport ("Error: The value for ChatroomNumMessages in the configuration file is illegal");
        return ERROR_FAILURE;
    }

    if (iMaxMessageLength < 32) {
        g_pReport->WriteReport ("Error: The value for ChatroomMaxMessageLength in the configuration file is illegal");
        return ERROR_FAILURE;
    }

    g_pReport->WriteReport ("Finished checking parameters from configuration");

    // Create new game engine object
    g_pReport->WriteReport ("Initializing GameEngine");

    g_pGameEngine = new GameEngine (
        pszDatabaseName, 
        pszHookLibrary,
        this,
        g_pReport,
        g_pPageSourceControl,
        &scConfig
        );

    if (g_pGameEngine == NULL) {
        goto OutOfMemory;
    }

    // Copy resource dir
    g_pszResourceDir = String::StrDup (pszResourceDir);
    if (g_pszResourceDir == NULL) {
        goto OutOfMemory;
    }

    // Initialize HtmlRenderer
    iErrCode = HtmlRenderer::Initialize();
    if (iErrCode != OK) {
        g_pReport->WriteReport ("The server is out of memory");
        return iErrCode;
    }

    // Create chatroom
    ChatroomConfig ccConfig;

    ccConfig.cchMaxSpeakerNameLen = MAX_EMPIRE_NAME_LENGTH;
    ccConfig.sTimeOut = sTimeOut;
    ccConfig.iMaxNumMessages = iMaxNumMessages;
    ccConfig.iMaxNumSpeakers = iMaxNumSpeakers;
    ccConfig.iMaxMessageLength = iMaxMessageLength;
    ccConfig.bPostSystemMessages = bPostSystemMessages;

    g_pChatroom = new Chatroom (ccConfig);
    if (g_pChatroom == NULL || g_pChatroom->Initialize() != OK) {
        goto OutOfMemory;
    }

    if (g_pGameEngine->Initialize() != OK) {
        g_pReport->WriteReport ("The Almonaster GameEngine could not be initialized successfully");
        goto Error;
    }

    g_pReport->WriteReport ("Finished initializing GameEngine");

    g_pReport->WriteReport ("Almonaster will now begin");
    g_pReport->WriteReport ("===================================================");

    return OK;

OutOfMemory:
    g_pReport->WriteReport ("An object allocation returned NULL in Almonaster::OnInitialize");

Error:
    
    g_pReport->WriteReport ("Almonaster_OnInitialize failed");

    return ERROR_FAILURE;
}

// A get will mostly be a request for an image file or the login screen
// We should only override requests for "/" or for the page source name
// followed by a "/"
int Almonaster::OnGet (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    // Look at the URI and override only /almonaster/ requests
    const char* pszUri = pHttpRequest->GetUri();

    if (String::StriCmp (m_pszUri1, pszUri) != 0 &&
        String::StriCmp (m_pszUri2, pszUri) != 0
        ) {

        // No override
        return OK;
    }

    return OnPost (pHttpRequest, pHttpResponse);
}


// Here we know that someone has POSTED information to our page source,
// so we need to reply with some page or other
int Almonaster::OnPost (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    // Find the right function to dispatch to
    PageId pageId = LOGIN;
    IHttpForm* pHttpForm = pHttpRequest->GetForm ("PageId");

    if (pHttpForm != NULL) {

        int iValue = pHttpForm->GetIntValue();
        if (iValue > MIN_PAGE_ID && iValue < MAX_PAGE_ID) {
            pageId = (PageId) iValue;
        }
    }

    // Enable chunked encoding, if wanted or if the page requested is a doc page
    if (m_bNoBuffering || pageId == SYSTEM_DOCUMENTATION || pageId == GAME_DOCUMENTATION) {
        pHttpResponse->SetNoBuffering();
    }

    // Call the function
    HtmlRenderer htmlRenderer (pageId, pHttpRequest, pHttpResponse);
    return htmlRenderer.Render();
}


int Almonaster::OnFinalize() {
    
    g_pReport->WriteReport ("Shutting down GameEngine");

    // Delete game engine object
    if (g_pGameEngine != NULL) {
        int iRefs = g_pGameEngine->Release();
        Assert (iRefs == 0);
        g_pGameEngine = NULL;
    }

    g_pReport->WriteReport ("Finished shutting down GameEngine");
    g_pReport->WriteReport ("Shutting down objects and cleaning up data");

    // Delete chatroom
    if (g_pChatroom != NULL) {
        delete g_pChatroom;
        g_pChatroom = NULL;
    }

    if (g_pszResourceDir != NULL) {
        OS::HeapFree (g_pszResourceDir);
        g_pszResourceDir = NULL;
    }

    g_pReport->WriteReport ("Finished shutting down objects and cleaning up data");
    g_pReport->WriteReport ("===================================================");

    return OK;
}

int Almonaster::OnAccessDenied (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    Variant vEmail;
    int iErrCode = g_pGameEngine->GetSystemProperty (SystemData::AdminEmail, &vEmail);
    const char* pszMail = vEmail.GetCharPtr();

    HttpStatusReason rReason = pHttpResponse->GetStatusCodeReason();

    pHttpResponse->WriteText (
        "<html>"\
        "<head><title>Access denied</title></head>"\
        "<body>"\
        "<center><h1>Access denied</h1></center><p>"
        );
    
    switch (rReason) {
        
    case HTTP_REASON_IPADDRESS_BLOCKED:
        
        pHttpResponse->WriteText (
            "Your IP address has been banned by the administrator. "
            );
        break;
        
    case HTTP_REASON_USER_AGENT_BLOCKED:
        
        pHttpResponse->WriteText (
            "You cannot use your web browser with Almonaster. Please download and install a more modern browser. "
            );
        break;

    case HTTP_REASON_GET_REFERER_BLOCKED:

        pHttpResponse->WriteText (
            "You cannot download images from Almonaster from outside the game pages. "
            );
        break;

    default:

        break;
    }

    pHttpResponse->WriteText (
        "Contact the "
        );

    if (iErrCode == OK && pszMail != NULL && *pszMail != '\0') {
        pHttpResponse->WriteText ("<a href=\"mailto:");
        pHttpResponse->WriteText (pszMail);
        pHttpResponse->WriteText ("\">administrator</a> ");
    } else {
        pHttpResponse->WriteText ("administrator ");
    }

    pHttpResponse->WriteText ("for more details."\

        "</body>"\
        "</html>"
        );

    return OK;
}

int Almonaster::OnError (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    // We're only interested in 403 - forbidden errors
    if (pHttpResponse->GetStatusCode() == HTTP_403) {
        return OnAccessDenied (pHttpRequest, pHttpResponse);
    }

    return OK;
}

int Almonaster::OnBasicAuthenticate (const char* pszLogin, const char* pszPassword, bool* pbAuthenticate) {
    
    *pbAuthenticate = false;
    return OK;
}

//
// IAlmonasterUIEventSink
//

int Almonaster::OnCreateEmpire (int iEmpireKey) {

    return HtmlRenderer::OnCreateEmpire (iEmpireKey);
}

int Almonaster::OnDeleteEmpire (int iEmpireKey) {

    return HtmlRenderer::OnDeleteEmpire (iEmpireKey);
}

int Almonaster::OnLoginEmpire (int iEmpireKey) {

    return HtmlRenderer::OnLoginEmpire (iEmpireKey);
}

int Almonaster::OnCreateGame (int iGameClass, int iGameNumber) {

    return HtmlRenderer::OnCreateGame (iGameClass, iGameNumber);
}

int Almonaster::OnCleanupGame (int iGameClass, int iGameNumber) {

    return HtmlRenderer::OnCleanupGame (iGameClass, iGameNumber);
}

int Almonaster::OnDeleteTournament (unsigned int iTournamentKey) {

    return HtmlRenderer::OnDeleteTournament (iTournamentKey);
}

int Almonaster::OnDeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey) {

    return HtmlRenderer::OnDeleteTournamentTeam (iTournamentKey, iTeamKey);
}