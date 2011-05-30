// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 1993-1996  Microsoft Corporation.  All Rights Reserved.
//
//  MODULE: service.h
//
//  AUTHOR: Craig Link

#ifndef _SERVICE_H
#define _SERVICE_H

#include <windows.h>

// name of the executable
#define SZAPPNAME            _T("AlajarSvc")
// internal name of the service
#define SZSERVICENAME        _T("Alajar")
// displayed name of the service
#define SZSERVICEDISPLAYNAME _T("Alajar")
// list of service dependencies - "dep1\0dep2\0\0"
#define SZDEPENDENCIES       _T("")

// Main.cpp must implement these functions
VOID Start();
VOID Stop();

// Main.cpp should call these functions when it finishes the
// respective actions
VOID ServiceStarted();
VOID ServiceStartAborted();

VOID ServiceStopped();

// Main.cpp can call this function to stop the service
VOID StopService();

#endif