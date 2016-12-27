// Event.h: interface for the Event class.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA  02111-1307, USA.

#if !defined(AFX_EVENT_H__A2793505_B31C_11D2_9F9D_0060083E8062__INCLUDED_)
#define AFX_EVENT_H__A2793505_B31C_11D2_9F9D_0060083E8062__INCLUDED_

#include "OS.h"

class OSAL_EXPORT Event {
protected:

#ifdef __LINUX__
    bool m_bInit;
    pthread_mutex_t m_Lock;
	pthread_cond_t m_hEvent;
#else if defined __WIN32__
	HANDLE m_hEvent;
#endif

public:

    Event();
    ~Event();
    
    int Initialize();

    int Signal();
    int Wait();
    int Wait (MilliSeconds iWait);
};

#endif // !defined(AFX_EVENT_H__A2793505_B31C_11D2_9F9D_0060083E8062__INCLUDED_)
