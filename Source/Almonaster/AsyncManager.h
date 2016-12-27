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

#pragma once

#include "Osal/Event.h"
#include "Osal/Thread.h"
#include "Osal/FifoQueue.h"

struct AsyncTask;
typedef int (THREAD_CALL *Fxn_QueryCallBack)(AsyncTask*);

struct AsyncTask
{
    Fxn_QueryCallBack pQueryCall;
    void* pArguments;
    TransactionIsolationLevel isoLevel;
};

class AsyncManager
{
private:
    Event m_eQueryEvent;
    Thread m_tLongRunningQueries;

    ThreadSafeFifoQueue<AsyncTask*> m_tsfqQueryQueue;

    static int THREAD_CALL StartAsyncTaskLoop(void* pVoid);
    int AsyncTaskLoop();

    Event m_eExitEvent;
    Thread m_tAvailability;

    static int THREAD_CALL StartAvailabilityLoop(void* pVoid);
    int AvailabilityLoop();

public:
    int Initialize();
    void Close();

    int QueueTask(Fxn_QueryCallBack pfxFunction, void* pVoid);
    int QueueTask(Fxn_QueryCallBack pfxFunction, void* pVoid, TransactionIsolationLevel isoLevel);
};

