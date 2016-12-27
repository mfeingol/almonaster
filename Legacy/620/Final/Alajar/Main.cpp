//
// Alajar 1.0:  A web server
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include "Alajar.h"

#include "Osal/Event.h"

#include <stdio.h>

class Sink : public IShutdownSink, public IStartupSink {
private:

    int m_iErrCode;
    Event& m_evShutdown;

    Sink& operator=(Sink& sinkCopy);

public:

    Sink (Event& evShutdown) : m_evShutdown (evShutdown), m_iNumRefs (1), m_iErrCode (OK) {}

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

static IHttpServer* g_pAlajar = NULL;

void CONTROL_CALL BreakHandler() {

    // Initiate a shutdown
    if (g_pAlajar != NULL) {
        g_pAlajar->Shutdown();
    }
}

int main() {

    Event evShutdown;
    Sink sSink (evShutdown);

    IConfigFile* pConfig = NULL;

    // Set ctrl-c handler
    int iErrCode = OS::SetBreakHandler (BreakHandler);
    if (iErrCode != OK) {
        printf ("Could not set the break handler for the process");
        goto Cleanup;
    }
    
    iErrCode = evShutdown.Initialize();
    if (iErrCode != OK) {
        printf ("Could not initialize an event object");
        goto Cleanup;
    }

    // Create new web server object
    iErrCode = AlajarCreateInstance (CLSID_HttpServer, IID_IHttpServer, (void**) &g_pAlajar);
    if (iErrCode != OK) {
        printf ("Could not create an instance of IHttpServer");
        goto Cleanup;
    }

    // Create new ConfigFile object
    iErrCode = AlajarCreateInstance (CLSID_ConfigFile, IID_IConfigFile, (void**) &pConfig);
    if (iErrCode != OK) {
        printf ("Could not create an instance of IConfigFile");
        goto Cleanup;
    }

    // Open the config file
    iErrCode = pConfig->Open ("Alajar.conf");
    if (iErrCode != OK) {
        printf ("Could not open Alajar.conf\n");
        goto Cleanup;
    }

    // Startup
    g_pAlajar->Start (pConfig, &sSink, &sSink);
    evShutdown.Wait();

    if (sSink.GetErrCode() != OK) {
        printf ("Alajar could not be started\n");
        goto Cleanup;
    }

    // Wait for server shutdown
    evShutdown.Wait();

Cleanup:

    // Release objects
    if (pConfig != NULL) {
        pConfig->Release();
    }

    // Terminate server
    if (g_pAlajar != NULL) {
        g_pAlajar->Release();
    }

    return 0;
}