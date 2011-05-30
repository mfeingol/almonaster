// Interface.cpp
//
//////////////////////////////////////////////////////////////////////
//
// Alajar 1.0:  a web server
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

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD

#include "HttpServer.h"

int AlajarCreateInstance (const Uuid& clsidClassId, const Uuid& iidInterface, void** ppObject) {

	if (clsidClassId == CLSID_HttpServer && iidInterface == IID_IHttpServer) {

		*ppObject = HttpServer::CreateInstance();
		return *ppObject != NULL ? OK : ERROR_FAILURE;
	}

	if (clsidClassId == CLSID_ConfigFile && iidInterface == IID_IConfigFile) {

		*ppObject = Config::CreateInstance();
		return *ppObject != NULL ? OK : ERROR_FAILURE;
	}

	*ppObject = NULL;
	return ERROR_NO_INTERFACE;
}

const Uuid CLSID_HttpServer = { 0x6538a8d5, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid CLSID_ConfigFile = { 0x6538a8d6, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };

const Uuid IID_ICachedFile = { 0x6538a8d7, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IFileCache = { 0x6538a8d8, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IReport = { 0x6538a8d9, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_ILog = { 0x6538a8da, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IConfigFile = { 0x6538a8db, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IPageSourceControl = { 0x6538a8dc, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IHttpForm = { 0x6538a8dd, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_ICookie = { 0x6538a8de, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IHttpRequest = { 0x6538a8df, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IHttpResponse = { 0x6538a8e0, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IHttpServer = { 0x6538a8e1, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IStartupSink = { 0x6538a8e2, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IShutdownSink = { 0x6538a8e3, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IPageSourceEnumerator = { 0x6538a8e4, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IPageSource = { 0x3c230791, 0x8cea, 0x11d3, { 0xa2, 0x40, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };