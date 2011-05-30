// DBConverter.cpp: implementation of the DBConverter class.
//
//////////////////////////////////////////////////////////////////////

#include "DBConverter.h"

#include "Osal/Algorithm.h"

#include <stdio.h>
#include <stdarg.h>

const Uuid My_CLSID_Database = { 0x6538a8ca, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid My_IID_IDatabase = { 0x6538a8d0, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };

void Output (const char* pszString, ...) {

    va_list args;
    va_start (args, pszString);

    vfprintf (stdout, pszString, args);
    fprintf (stdout, "\n");

    va_end (args);
}


int DBConverter::TemplateNameHashValue::GetHashValue (const char* pszString, unsigned int iNumBuckets, const void* pvHashHint) {
    return Algorithm::GetStringHashValue (pszString, iNumBuckets, true);
}

bool DBConverter::TemplateNameEquals::Equals (const char* pszLeft, const char* pszRight, const void* pvEqualsHint) {
    return String::StriCmp (pszLeft, pszRight) == 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

typedef int (*Fxn_DatabaseCreateInstance) (const Uuid& clsidClassId, const Uuid& iidInterface, void** ppObject);

DBConverter::DBConverter() : m_htTemplates (NULL, NULL) {

    m_pSrcDatabase = NULL;
    m_pDestDatabase = NULL;

    m_iNumTablesConverted = 0;

    m_bDeleteDestTables = false;
    m_bImportTemplatesIfMissing = false;
}

DBConverter::~DBConverter() {

    char* pszName;
    HashTableIterator<char*, char*> htiTemplate;
    while (m_htTemplates.GetNextIterator (&htiTemplate)) {
        if (m_htTemplates.Delete (&htiTemplate, &pszName, NULL)) {
            OS::HeapFree (pszName);
        }
    }

    if (m_pSrcDatabase != NULL) {
        Output ("Shutting down source database");
        m_pSrcDatabase->Release();
        Output ("Finished shutting down source database");
    }

    if (m_pDestDatabase != NULL) {
        Output ("Shutting down destination database");
        m_pDestDatabase->Release();
        Output ("Finished shutting down destination database");
    }

    if (m_lSrcDatabase.IsOpen()) {
        m_lSrcDatabase.Close();
    }

    if (m_lDestDatabase.IsOpen()) {
        m_lDestDatabase.Close();
    }
}

int DBConverter::Open (ConfigFile* pcfConfig) {

    int iErrCode;
    char* pszRhs;
    Fxn_DatabaseCreateInstance pDatabaseCreateInstance;

    if (!m_htTemplates.Initialize (1000)) {
        Output ("Out of memory initialize hash table");
        return ERROR_OUT_OF_MEMORY;
    }

    Output ("Reading data from the config file");

    // Load source database
    if (pcfConfig->GetParameter ("SrcDatabase", &pszRhs) != OK || pszRhs == NULL) {
        Output ("Could not read SrcDatabase parameter from config file");
        return ERROR_FAILURE;
    }
    if (m_lSrcDatabase.Open (pszRhs) != OK) {
        Output ("Could not open source database library");
        return ERROR_FAILURE;
    }

    pDatabaseCreateInstance = (Fxn_DatabaseCreateInstance) m_lSrcDatabase.GetExport ("DatabaseCreateInstance");
    if (pDatabaseCreateInstance == NULL) {
        Output ("Could not find DatabaseCreateInstance export in source database library");
        return ERROR_FAILURE;
    }

    if (pDatabaseCreateInstance (My_CLSID_Database, My_IID_IDatabase, (void**) &m_pSrcDatabase) != OK ||
        m_pSrcDatabase == NULL) {
        Output ("Could not create an instance of the source database library");
        return ERROR_FAILURE;
    }

    // Load destination database
    if (pcfConfig->GetParameter ("DestDatabase", &pszRhs) != OK || pszRhs == NULL) {
        Output ("Could not read DestDatabase parameter from config file");
        return ERROR_FAILURE;
    }
    if (m_lDestDatabase.Open (pszRhs) != OK) {
        Output ("Could not open destination database library");
        return ERROR_FAILURE;
    }

    pDatabaseCreateInstance = (Fxn_DatabaseCreateInstance) m_lDestDatabase.GetExport ("DatabaseCreateInstance");
    if (pDatabaseCreateInstance == NULL) {
        Output ("Could not find DatabaseCreateInstance export in destination database library");
        return ERROR_FAILURE;
    }

    if (pDatabaseCreateInstance (My_CLSID_Database, My_IID_IDatabase, (void**) &m_pDestDatabase) != OK ||
        m_pDestDatabase == NULL) {
        Output ("Could not create an instance of the destination database library");
        return ERROR_FAILURE;
    }

    // Get source path
    if (pcfConfig->GetParameter ("SrcPath", &pszRhs) != OK || pszRhs == NULL) {
        Output ("Could not read SrcPath parameter from config file");
        return ERROR_FAILURE;
    }
    m_strSrcDirectory = pszRhs;

    if (!File::DoesDirectoryExist (m_strSrcDirectory)) {
        Output ("The source path does not exist");
        return ERROR_FAILURE;
    }

    // Get dest path
    if (pcfConfig->GetParameter ("DestPath", &pszRhs) != OK || pszRhs == NULL) {
        Output ("Could not read DestPath parameter from config file");
        return ERROR_FAILURE;
    }
    m_strDestDirectory = pszRhs;

    if (!File::DoesDirectoryExist (m_strDestDirectory)) {
        Output ("The destination path does not exist");
        return ERROR_FAILURE;
    }

    // Get DeleteDestTables
    if (pcfConfig->GetParameter ("DeleteDestDatabaseTables", &pszRhs) != OK || pszRhs == NULL) {
        Output ("Could not read DeleteDestDatabaseTables parameter from config file");
        return ERROR_FAILURE;
    }
    m_bDeleteDestTables = atoi (pszRhs) != 0;

    // Get ImportTemplatesIfMissing
    if (pcfConfig->GetParameter ("ImportTemplatesIfMissing", &pszRhs) != OK || pszRhs == NULL) {
        Output ("Could not read ImportTemplatesIfMissing parameter from config file");
        return ERROR_FAILURE;
    }
    m_bImportTemplatesIfMissing = atoi (pszRhs) != 0;

    Output ("Finished reading data from the config file");
    Output ("Initializing source database");

    // Initialize databases
    iErrCode = m_pSrcDatabase->Initialize (m_strSrcDirectory, DATABASE_CHECK);
    if (iErrCode != OK && iErrCode != WARNING) {
        Output ("The source database could not be initialized");
        return iErrCode;
    }

    Output ("Finished initializing source database");
    Output ("Initializing destination database");

    iErrCode = m_pDestDatabase->Initialize (m_strDestDirectory, DATABASE_CHECK | DATABASE_WRITETHROUGH);
    if (iErrCode != OK && iErrCode != WARNING) {
        Output ("The destination database could not be initialized");
        return iErrCode;
    }

    // Make sure that the src database isn't empty
    if (m_pSrcDatabase->GetNumTables() == 0) {
        Output ("The source database cannot be empty");
        return ERROR_FAILURE;
    }

    // Get template list
    if (pcfConfig->GetParameter ("Templates", &pszRhs) != OK || pszRhs == NULL) {
        Output ("Could not read Templates parameter from config file");
        return ERROR_FAILURE;
    }
    
    Output ("Finished initializing destination database");
    Output ("Parsing templates");

    if (strcmp (pszRhs, "*") == 0) {

        // Load up list with template names
        ITemplateEnumerator* pTemplateEnumerator = m_pSrcDatabase->GetTemplateEnumerator();
        if (pTemplateEnumerator == NULL) {
            Assert (false);
            Output ("A template enumerator could not be obtained from the source database");
            return ERROR_FAILURE;
        }

        unsigned int i, iNumTemplates = pTemplateEnumerator->GetNumTemplates();
        const char** ppszTemplateName = pTemplateEnumerator->GetTemplateNames();

        char* pszName;

        for (i = 0; i < iNumTemplates; i ++) {

            pszName = String::StrDup (ppszTemplateName[i]);
            m_htTemplates.Insert (pszName, pszName);
        }

        pTemplateEnumerator->Release();

    } else {

        const char szChar = ';', * pszSeparator = ";";

        char* pszList, * pszTemp, * pszName;

        // Parse list
        pszList = pszRhs;
        while (true) {

            pszTemp = strstr (pszList, pszSeparator);

            if (pszTemp == NULL) {

                if (*pszList != '\0') {
                    pszName = String::StrDup (pszList);
                    m_htTemplates.Insert (pszName, pszName);
                }
                break;

            } else {

                if (*pszList != szChar) {
                    *pszTemp = '\0';
                    pszName = String::StrDup (pszList);
                    m_htTemplates.Insert (pszName, pszName);
                    *pszTemp = szChar;
                }
                pszList = pszTemp + 1;
            }
        }
    }

    if (m_htTemplates.GetNumElements() == 0) {
        Assert (false);
        Output ("No valid template names were specified");
    }

    Output ("Finished parsing templates");

    // Okay, we're open
    return OK;
}


int DBConverter::Convert() {

    ITableEnumerator* pTableEnum = NULL;
    ITemplate* pSrcTableTemplate = NULL;

    TemplateDescription ttSrcTableTemplateDescription;
    HashTableIterator<char*, char*> htiTemplate;

    int iErrCode = OK;
    unsigned int i, iNumTables;
    const char** ppszTableName;

    Output ("Beginning database conversion");
    Output ("=============================");

    // Delete all destination database tables?
    if (m_bDeleteDestTables) {

        pTableEnum = m_pDestDatabase->GetTableEnumerator();
        if (pTableEnum == NULL) {
            Assert (false);
            Output ("The destination database could not provide a table enumerator");
            iErrCode = ERROR_FAILURE;
            goto Cleanup;
        }
        
        iNumTables = pTableEnum->GetNumTables();
        ppszTableName = pTableEnum->GetTableNames();
        
        for (i = 0; i < iNumTables; i ++) {
            
            iErrCode = m_pDestDatabase->DeleteTable (ppszTableName[i]);
            if (iErrCode != OK) {
                Assert (false);
                Output ("Could not delete table %s from the destination database", ppszTableName[i]);
                goto Cleanup;
            }
        }

        pTableEnum->Release();
        pTableEnum = NULL;
    }

    // Enumerate all src database tables
    pTableEnum = m_pSrcDatabase->GetTableEnumerator();
    if (pTableEnum == NULL) {
        Assert (false);
        Output ("The source database could not provide a table enumerator");
        iErrCode = ERROR_FAILURE;
        goto Cleanup;
    }

    iNumTables = pTableEnum->GetNumTables();
    ppszTableName = pTableEnum->GetTableNames();

    if (iNumTables == 0) {
        Assert (false);
        Output ("The source database's table enumerator did not contain any tables");
        iErrCode = ERROR_FAILURE;
        goto Cleanup;
    }

    // Go table by table
    for (i = 0; i < iNumTables; i ++) {

        // Get table's template and description
        iErrCode = m_pSrcDatabase->GetTemplateForTable (ppszTableName[i], &pSrcTableTemplate);
        if (iErrCode != OK) {
            Assert (false);
            Output ("The source database could not provide a template for the table %s", ppszTableName[i]);
            goto Cleanup;
        }

        iErrCode = pSrcTableTemplate->GetDescription (&ttSrcTableTemplateDescription);
        if (iErrCode != OK) {
            Assert (false);
            Output ("The source database could not provide a template description for the table %s", ppszTableName[i]);
            goto Cleanup;
        }

        // Does dest database have this template?
        if (m_bImportTemplatesIfMissing &&
            !m_pDestDatabase->DoesTemplateExist (ttSrcTableTemplateDescription.Name)) {

            iErrCode = m_pDestDatabase->CreateTemplate (ttSrcTableTemplateDescription);
            if (iErrCode != OK) {
                Assert (false);
                Output ("The template %s could not be created in the destination database; the error was %i", 
                    ttSrcTableTemplateDescription.Name, iErrCode);
                goto Cleanup;
            }
        }

        // Is template specified for migration?
        if (m_htTemplates.FindFirst (ttSrcTableTemplateDescription.Name, &htiTemplate)) {

            Output ("%i of %i - %s", i + 1, iNumTables, ppszTableName[i]);

            // Migrate table!
            iErrCode = m_pDestDatabase->ImportTable (m_pSrcDatabase, ppszTableName[i]);
            if (iErrCode != OK) {
                Assert (false);
                Output ("The table %s could not be imported into the destination database; the error was %i", 
                    ppszTableName[i], iErrCode);
                goto Cleanup;
            }
        }

        if (pSrcTableTemplate != NULL) {
            pSrcTableTemplate->Release();
            pSrcTableTemplate = NULL;
        }

        m_iNumTablesConverted ++;
    }

    Output ("============================");
    Output ("Finished database conversion");

    iErrCode = m_pDestDatabase->Check();
    if (iErrCode == OK) {
        Output ("Destination database verification was successful");
    } else {
        Assert (false);
        Output ("Destination database verification failed");
    }

Cleanup:

    if (pTableEnum != NULL) {
        pTableEnum->Release();
    }

    if (pSrcTableTemplate != NULL) {
        pSrcTableTemplate->Release();
    }

    return iErrCode;
}