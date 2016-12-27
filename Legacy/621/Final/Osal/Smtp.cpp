// Smtp.cpp: implementation of SMTP client functionality
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

#define OSAL_BUILD
#include "Smtp.h"
#include "String.h"
#include "Time.h"
#undef OSAL_BUILD

#include <stdio.h>

#define TRACE_SMTP_PROTOCOL

SmtpClient::SmtpClient (const char* pszServer, unsigned short siPort) {

    m_pszServer = String::StrDup (pszServer);
    m_siPort = siPort;
    m_bConnected = false;
}

SmtpClient::~SmtpClient() {
    OS::HeapFree (m_pszServer);
}

int SmtpClient::Connect() {

    int iErrCode;

    // Connect
    iErrCode = m_socket.Open();
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = m_socket.Connect (m_pszServer, m_siPort);
    if (iErrCode != OK)
        return iErrCode;

    // Send EHLO
    size_t cbSent, cbRecvd;
    char pszRecvBuffer [128];
    char pszSendBuffer [160];

    char pszHostName [128];
    iErrCode = Socket::GetOurHostName (pszHostName, countof (pszHostName));
    if (iErrCode != OK)
        return iErrCode;

    sprintf (pszSendBuffer, "EHLO %s\r\n", pszHostName);
#ifdef TRACE_SMTP_PROTOCOL
    printf ("Us: %s", pszSendBuffer);
#endif
    iErrCode = m_socket.Send (pszSendBuffer, &cbSent);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = m_socket.Recv (pszRecvBuffer, countof (pszRecvBuffer) - 1, &cbRecvd);
    if (iErrCode != OK)
        return iErrCode;
    pszRecvBuffer [cbRecvd] = '\0';
#ifdef TRACE_SMTP_PROTOCOL
    printf ("Them: %s", pszRecvBuffer);
#endif

    if (cbRecvd < 3 || strncmp (pszRecvBuffer, "220", 3) != 0) {
        iErrCode = ERROR_FAILURE;
        return iErrCode;
    }

    m_bConnected = true;

    return iErrCode;
}

void SmtpClient::Disconnect() {

    if (m_bConnected) {

        int iErrCode;
        size_t cbSent, cbRecvd;
        char pszRecvBuffer [128];

#ifdef TRACE_SMTP_PROTOCOL
        printf ("Us: QUIT\r\n");
#endif
        iErrCode = m_socket.Send ("QUIT\r\n", &cbSent);
        if (iErrCode == OK) {

            iErrCode = m_socket.Recv (pszRecvBuffer, countof (pszRecvBuffer) - 1, &cbRecvd);
            if (iErrCode == OK) {
                pszRecvBuffer [cbRecvd] = '\0';
#ifdef TRACE_SMTP_PROTOCOL
                printf ("Them: %s", pszRecvBuffer);
#endif
            }
        }

        m_bConnected = false;
    }

    if (m_socket.IsConnected()) {
        m_socket.Close();
    }
}

int SmtpClient::SendMessage (const SmtpMessage& message) {

    int iErrCode = OK;

    size_t cbSent, cbRecvd, cchLen;
    char pszSendBuffer [256];
    char pszRecvBuffer [128];

    if (!m_bConnected) {
        Assert (false);
        return ERROR_FAILURE;
    }

    // MAIL FROM
    cchLen = String::StrLen (message.From);
    if (cchLen < 1 || cchLen > 128) {
        Assert (false);
        return ERROR_FAILURE;
    }
    sprintf (pszSendBuffer, "MAIL FROM:<%s>\r\n", message.From);
#ifdef TRACE_SMTP_PROTOCOL
    printf ("Us: %s", pszSendBuffer);
#endif
    iErrCode = m_socket.Send (pszSendBuffer, &cbSent);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = m_socket.Recv (pszRecvBuffer, countof (pszRecvBuffer) - 1, &cbRecvd);
    if (iErrCode != OK)
        return iErrCode;
    pszRecvBuffer [cbRecvd] = '\0';
#ifdef TRACE_SMTP_PROTOCOL
    printf ("Them: %s", pszRecvBuffer);
#endif

    if (cbRecvd < 3 || strncmp (pszRecvBuffer, "250", 3) != 0) {
        iErrCode = ERROR_FAILURE;
        return iErrCode;
    }

    // RCPT TO:
    cchLen = String::StrLen (message.To);
    if (cchLen < 1 || cchLen > 128) {
        Assert (false);
        return ERROR_FAILURE;
    }
    sprintf (pszSendBuffer, "RCPT TO:<%s>\r\n", message.To);
#ifdef TRACE_SMTP_PROTOCOL
    printf ("Us: %s", pszSendBuffer);
#endif
    iErrCode = m_socket.Send (pszSendBuffer, &cbSent);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = m_socket.Recv (pszRecvBuffer, countof (pszRecvBuffer) - 1, &cbRecvd);
    if (iErrCode != OK)
        return iErrCode;
    pszRecvBuffer [cbRecvd] = '\0';
#ifdef TRACE_SMTP_PROTOCOL
    printf ("Them: %s", pszRecvBuffer);
#endif

    if (cbRecvd < 3 || strncmp (pszRecvBuffer, "250", 3) != 0) {
        iErrCode = ERROR_FAILURE;
        return iErrCode;
    }

    // DATA
#ifdef TRACE_SMTP_PROTOCOL
    printf ("Us: DATA\r\n");
#endif
    iErrCode = m_socket.Send ("DATA\r\n", &cbSent);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = m_socket.Recv (pszRecvBuffer, countof (pszRecvBuffer) - 1, &cbRecvd);
    if (iErrCode != OK)
        return iErrCode;
    pszRecvBuffer [cbRecvd] = '\0';
#ifdef TRACE_SMTP_PROTOCOL
    printf ("Them: %s", pszRecvBuffer);
#endif

    if (cbRecvd < 3 || (strncmp (pszRecvBuffer, "354", 3) != 0 && strncmp (pszRecvBuffer, "250", 3) != 0)) {
        iErrCode = ERROR_FAILURE;
        return iErrCode;
    }

    // Headers
    char pszDate [OS::MaxSmtpDateLength];
    Time::GetSmtpDateString (pszDate);

    char* pszHeaders = (char*) StackAlloc (
        256 + 
        String::StrLen (message.SenderName) +
        String::StrLen (message.From) +
        String::StrLen (message.To) +
        String::StrLen (message.Subject) + 
        String::StrLen (pszDate)
        );

    sprintf (pszHeaders,
        "From: \"%s\" <%s>\r\n"
        "To: <%s>\r\n"
        "Subject: %s\r\n"
        "Date: %s\r\n"
        "X-Mailer: OSAL 1.0\r\n"
        "Content-Type: text/plain; charset=\"us-ascii\"\r\n"
        "\r\n",
        message.SenderName,
        message.From,
        message.To,
        message.Subject,
        pszDate
        );

#ifdef TRACE_SMTP_PROTOCOL
    printf ("Us: %s", pszHeaders);
#endif
    iErrCode = m_socket.Send (pszHeaders, &cbSent);
    if (iErrCode != OK)
        return iErrCode;

    // Body
#ifdef TRACE_SMTP_PROTOCOL
    printf ("%s\r\n.\r\n", message.Body);
#endif
    iErrCode = m_socket.Send (message.Body, &cbSent);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = m_socket.Send ("\r\n.\r\n", &cbSent);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = m_socket.Recv (pszRecvBuffer, countof (pszRecvBuffer) - 1, &cbRecvd);
    if (iErrCode != OK)
        return iErrCode;
    pszRecvBuffer [cbRecvd] = '\0';
#ifdef TRACE_SMTP_PROTOCOL
    printf ("Them: %s", pszRecvBuffer);
#endif

    if (cbRecvd < 3 || strncmp (pszRecvBuffer, "250", 3) != 0) {
        iErrCode = ERROR_FAILURE;
        return iErrCode;
    }

    return iErrCode;
}