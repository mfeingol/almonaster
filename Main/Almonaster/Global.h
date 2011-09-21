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

#include "Osal/Library.h"

#include "Alajar.h"
#include "SqlDatabase.h"
#include "AlmonasterEventSink.h"
#include "CChatroom.h"
#include "AsyncManager.h"
#include "GameEngineConstants.h"

class Global
{
private:
    IHttpServer* m_pHttpServer;
    IReport* m_pReport;
    IConfigFile* m_pConfig;
    ILog* m_pLog;
    IPageSourceControl* m_pPageSourceControl;
    IFileCache* m_pFileCache;

    char m_pszResourceDir[OS::MaxFileNameLength];

    Library m_libDatabase;
    IDatabase* m_pDatabase;

    AlmonasterEventSink m_eventSink;
    Chatroom m_cChatroom;
    AsyncManager m_asyncManager;

    unsigned int m_iRootKey;
    unsigned int m_iGuestKey;

    int InitializeDatabase(const char* pszLibDatabase, const Uuid& uuidDatabaseClsid, const char* pszDatabaseConnectionString);
    int InitializeState();
    int InitializeChatroom();
    int InitializeScoringSystems();

public:
    Global();

    void TlsOpenConnection();
    int TlsCommitTransaction();
    void TlsCloseConnection();

    int Initialize(IHttpServer* pHttpServer, IPageSourceControl* pPageSourceControl);
    void Close();

    IDatabase* GetDatabase() { return m_pDatabase; }

    IAlmonasterEventSink* GetEventSink() { return &m_eventSink; }
    AsyncManager* GetAsyncManager() { return &m_asyncManager; }
    Chatroom* GetChatroom() { return &m_cChatroom; }

    IHttpServer* GetHttpServer() { return m_pHttpServer; }
    IPageSourceControl* GetPageSourceControl() { return m_pPageSourceControl; }
    IConfigFile* GetConfigFile() { return m_pConfig; }
    IFileCache* GetFileCache() { return m_pFileCache; }
    IReport* GetReport() { return m_pReport; }

    const char* GetResourceDir() { return m_pszResourceDir; }
    unsigned int GetRootKey() { return m_iRootKey; }
    unsigned int GetGuestKey() { return m_iGuestKey; }
};

extern Global global;

#define TRACE_ERROR(iErrCode)                                                               \
    char* pszError = (char*)StackAlloc(512 * sizeof(char));                                 \
    sprintf(pszError, "Error %i occurred in %s line %i", iErrCode, __FILE__, __LINE__);     \
    global.GetReport()->WriteReport(pszError);

#define GOTO_CLEANUP_ON_ERROR(iErrCode)                                                     \
    if (iErrCode != OK)                                                                     \
    {                                                                                       \
        TRACE_ERROR(iErrCode);                                                              \
        goto Cleanup;                                                                       \
    }                                                                                       

#define RETURN_ON_ERROR(iErrCode)                                                           \
    if (iErrCode != OK)                                                                     \
    {                                                                                       \
        TRACE_ERROR(iErrCode);                                                              \
        return iErrCode;                                                                    \
    }

class AutoFreeData
{
    static Variant* g_pvData;
    static Variant** g_ppvData;

private:
    Variant*& m_pvData;
    Variant**& m_ppvData;

public:
    AutoFreeData(Variant**& ppvData) : m_pvData(g_pvData), m_ppvData(ppvData)
    {
    }

    AutoFreeData(Variant*& pvData) : m_pvData(pvData), m_ppvData(g_ppvData)
    {
    }

    ~AutoFreeData()
    {
        if (m_ppvData)
        {
            t_pCache->FreeData(m_ppvData);
            m_ppvData = NULL;
        }

        if (m_pvData)
        {
            t_pCache->FreeData(m_pvData);
            m_pvData = NULL;
        }
    }
};

class AutoFreeKeys
{
private:
    unsigned int*& m_piKeys;

public:
    AutoFreeKeys(unsigned int*& piKeys) : m_piKeys(piKeys)
    {
    }

    ~AutoFreeKeys()
    {
        if (m_piKeys)
        {
            t_pCache->FreeKeys(m_piKeys);
            m_piKeys = NULL;
        }
    }
};

template <class T> class AutoFreeArrayOfArrays
{
private:
    T**& m_ppArray;
    unsigned int& m_iSize;

public:
    AutoFreeArrayOfArrays(T**& ppArray, unsigned int& iSize) : m_ppArray(ppArray), m_iSize(iSize)
    {
    }

    ~AutoFreeArrayOfArrays()
    {
        if (m_ppArray)
        {
            for (unsigned int i = 0; i < m_iSize; i ++)
            {
                if (m_ppArray[i])
                {
                    delete [] m_ppArray[i];
                    m_ppArray[i] = NULL;
                }
            }
        }
    }
};

class AutoFreeArrayOfKeys
{
private:
    unsigned int**& m_ppArray;
    unsigned int& m_iSize;

public:
    AutoFreeArrayOfKeys(unsigned int**& ppArray, unsigned int& iSize) : m_ppArray(ppArray), m_iSize(iSize)
    {
    }

    ~AutoFreeArrayOfKeys()
    {
        if (m_ppArray)
        {
            for (unsigned int i = 0; i < m_iSize; i ++)
            {
                if (m_ppArray[i])
                {
                    t_pCache->FreeKeys(m_ppArray[i]);
                    m_ppArray[i] = NULL;
                }
            }
        }
    }
};

class AutoFreeArrayOfData
{
private:
    Variant**& m_ppArray;
    unsigned int& m_iSize;

public:
    AutoFreeArrayOfData(Variant**& ppArray, unsigned int& iSize) : m_ppArray(ppArray), m_iSize(iSize)
    {
    }

    ~AutoFreeArrayOfData()
    {
        if (m_ppArray)
        {
            for (unsigned int i = 0; i < m_iSize; i ++)
            {
                if (m_ppArray[i])
                {
                    t_pCache->FreeData(m_ppArray[i]);
                    m_ppArray[i] = NULL;
                }
            }
        }
    }
};