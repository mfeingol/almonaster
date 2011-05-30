#include <stdio.h>

#include "Service.h"
#include "Alajar.h"
#include "Osal/Event.h"
#include "Osal/Thread.h"
#include "Osal/String.h"

class Sink : public IShutdownSink, public IStartupSink {
private:

    int m_iErrCode;
    Event& m_evShutdown;
    Sink (Event& evShutdown) : m_evShutdown (evShutdown), m_iNumRefs (1), m_iErrCode (OK) {}
    Sink& operator=(Sink& sinkCopy);

public:

    static Sink* CreateInstance (Event& evShutdown) {
        return new Sink (evShutdown);
    }

    int GetErrCode() {
        return m_iErrCode;
    }

    // IHttpServerShutdownSink
    IMPLEMENT_TWO_INTERFACES (IShutdownSink, IStartupSink);
    
    void OnStartup (int iErrCode) {
        m_iErrCode = iErrCode;
        m_evShutdown.Signal();
    }

    void OnShutdown (int iErrCode) {
        m_iErrCode = iErrCode;
        m_evShutdown.Signal();
    }
};

Sink* g_pSink = NULL;
IConfigFile* g_pConfig = NULL;
IHttpServer* g_pAlajar = NULL;
Event g_evShutdown;
Thread g_tStopWatch;

int THREAD_CALL StopWatch (void* pVoid) {

    g_evShutdown.Wait();

    // A pagesource shut down the server.
    // Tell the service about it
    StopService();

    return 0;
}

VOID Start() {

    // Create new web server object
    int iErrCode = AlajarCreateInstance (CLSID_HttpServer, IID_IHttpServer, (void**) &g_pAlajar);
    if (iErrCode != OK) {
        OS::Alert ("Could not create an instance of IHttpServer");
        ServiceStartAborted();
        return;
    }

    // Create new ConfigFile object
    iErrCode = AlajarCreateInstance (CLSID_ConfigFile, IID_IConfigFile, (void**) &g_pConfig);
    if (iErrCode != OK) {
        
        g_pAlajar->Release();
        g_pAlajar = NULL;

        OS::Alert ("Could not create an instance of IConfigFile");
        ServiceStartAborted();
        return;
    }

    // Get the working directory
    char pszPath [OS::MaxFileNameLength];
    iErrCode = OS::GetApplicationDirectory (pszPath);

    if (iErrCode != OK) {

        g_pAlajar->Release();
        g_pAlajar = NULL;

        OS::Alert ("Could not read the application directory");
        ServiceStartAborted();
        return;
    }

    if (!SetCurrentDirectory (pszPath)) {

        g_pAlajar->Release();
        g_pAlajar = NULL;

        OS::Alert ("Could not set the current directory");
        ServiceStartAborted();
        return;
    }

    // Open the config file
    strcat (pszPath, "/Alajar.conf");
    iErrCode = g_pConfig->Open (pszPath);
    if (iErrCode != OK) {

        g_pAlajar->Release();
        g_pAlajar = NULL;
        g_pConfig->Release();
        g_pConfig = NULL;

        OS::Alert ("Could not open Alajar.conf");
        ServiceStartAborted();
        return;
    }

    // Create an event sink
    g_pSink = Sink::CreateInstance (g_evShutdown);

    // Startup
    g_pAlajar->Start (g_pConfig, g_pSink, g_pSink);
    g_evShutdown.Wait();

    if (g_pSink->GetErrCode() != OK) {
        
        char pszAlert [128];
        snprintf (
            pszAlert, 
            sizeof (pszAlert), 
            "Alajar could not be started. The error code was 0x%x",
            g_pSink->GetErrCode()
            );

        g_pAlajar->Release();
        g_pAlajar = NULL;
        g_pConfig->Release();
        g_pConfig = NULL;
        g_pSink->Release();
        g_pSink = NULL;

        OS::Alert (pszAlert);
        ServiceStartAborted();
        return;
    }

    g_tStopWatch.Start (StopWatch, NULL);

    ServiceStarted();
}

VOID Stop() {

    // Nuke the stopwatch thread
    g_tStopWatch.Terminate();

    if (g_pAlajar != NULL) {

        // Shutdown the server
        g_pAlajar->Shutdown();
        
        // Wait for server shutdown
        g_evShutdown.Wait();
        
        // Terminate server
        g_pAlajar->Release();
        g_pAlajar = NULL;
        
        // Release objects
        g_pConfig->Release();
        g_pConfig = NULL;
        
        g_pSink->Release();
        g_pSink = NULL;
    }

    ServiceStopped();
}