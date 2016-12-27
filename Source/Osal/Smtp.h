// Smtp.h: interface for SMTP client functionality.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include "Socket.h"

struct OSAL_EXPORT SmtpMessage {

    const char* Subject;
    const char* SenderName;
    const char* To;
    const char* From;
    const char* ReplyTo;
    const char* Body;
};

class OSAL_EXPORT SmtpClient {
private:

    char* m_pszServer;
    unsigned short m_siPort;

    bool m_bConnected;
    Socket m_socket;

public:

    SmtpClient (const char* pszServer, unsigned short siPort);
    ~SmtpClient();

    int Connect();
    void Disconnect();

    int SendMessage (const SmtpMessage& message);
};