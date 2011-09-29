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

#include "AsyncManager.h"
#include "Global.h"
#include "GameEngine.h"

int AsyncManager::Initialize()
{
    int iErrCode;
    bool bInit;

    iErrCode = m_eQueryEvent.Initialize();
    Assert(iErrCode == OK);

    bInit = m_tsfqQueryQueue.Initialize();
    Assert(bInit);

    iErrCode = m_tLongRunningQueries.Start(StartAsyncTaskLoop, this);
    Assert(iErrCode == OK);

    iErrCode = m_eExitEvent.Initialize();
    Assert(iErrCode == OK);

    iErrCode = m_tAvailability.Start(StartAvailabilityLoop, this);
    Assert(iErrCode == OK);

    global.GetReport()->WriteReport ("GameEngine started asynchronous tasks");
    return OK;
}

void AsyncManager::Close()
{
    // Stop long running query processor
    if (m_tsfqQueryQueue.IsInitialized() && m_tLongRunningQueries.IsAlive())
    {
        bool push = m_tsfqQueryQueue.Push(NULL);
        Assert(push);
        m_eQueryEvent.Signal();
        m_tLongRunningQueries.WaitForTermination();
    }

    if (m_tAvailability.IsAlive())
    {
        m_eExitEvent.Signal();
        m_tAvailability.WaitForTermination();
    }
}

int AsyncManager::StartAsyncTaskLoop(void* pVoid)
{
    return ((AsyncManager*)pVoid)->AsyncTaskLoop();
}

int AsyncManager::AsyncTaskLoop()
{
    AsyncTask* plrqMessage;
    bool bExit = false;

    while (!bExit)
    {
        // Wait for action
        m_eQueryEvent.Wait();

        // Process messages
        while (true)
        {
            // Lock queue and get message
            if (!m_tsfqQueryQueue.Pop (&plrqMessage)) {
                // Back to sleep
                break;
            }

            // Process message
            if (plrqMessage == NULL)
            {
                // Shutting down
                bExit = true;
                break;
            }

            global.TlsOpenConnection();

            int iErrCode = plrqMessage->pQueryCall(plrqMessage);
            if (iErrCode == OK)
            {
                global.TlsCommitTransaction();
            }

            delete plrqMessage;
            global.TlsCloseConnection();
        }
    }

    return OK;
}

int AsyncManager::QueueTask(Fxn_QueryCallBack pfxnFunction, void* pVoid) {

    // Build the message
    AsyncTask* pMessage = new AsyncTask;
    Assert(pMessage);
    pMessage->pArguments = pVoid;
    pMessage->pQueryCall = pfxnFunction;

    // Push message into the queue
    bool ret = m_tsfqQueryQueue.Push(pMessage);
    Assert(ret);

    // Signal the event
    m_eQueryEvent.Signal();

    return OK;
}

int AsyncManager::StartAvailabilityLoop(void* pVoid)
{
    return ((AsyncManager*)pVoid)->AvailabilityLoop();
}

int AsyncManager::AvailabilityLoop()
{
    GameEngine gameEngine;
    while(true)
    {
        // Best effort
        int iErrCode = gameEngine.WriteAvailability();
        TRACE_ON_ERROR(iErrCode);

        iErrCode = m_eExitEvent.Wait(60 * 1000);
        if (iErrCode == WARNING)
        {
            // Keep going
            continue;
        }
        
        if (iErrCode == OK)
        {
            // Exiting
            break;
        }

        // Bad error
        Assert(false);
        break;
    }

    return OK;
}