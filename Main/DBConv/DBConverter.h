// DBConverter.h: interface for the DBConverter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBCONVERTER_H__637CBFB0_1A75_40EC_8CED_FDECCCD15964__INCLUDED_)
#define AFX_DBCONVERTER_H__637CBFB0_1A75_40EC_8CED_FDECCCD15964__INCLUDED_

#include "Osal/ConfigFile.h"
#include "Osal/Library.h"
#include "Osal/HashTable.h"

#include "Database.h"

class DBConverter {
private:

    Library m_lSrcDatabase;
    Library m_lDestDatabase;

    IDatabase* m_pSrcDatabase;
    IDatabase* m_pDestDatabase;

    String m_strSrcDirectory;
    String m_strDestDirectory;

    bool m_bDeleteDestTables;
    bool m_bImportTemplatesIfMissing;

    unsigned int m_iNumTablesConverted;

    // Hash table declarations
    class TemplateNameHashValue {
    public:
        static int GetHashValue (const char* pszString, unsigned int iNumBuckets, const void* pvHashHint);
    };
    
    class TemplateNameEquals {
    public:
        static bool Equals (const char* pszLeft, const char* pszRight, const void* pvEqualsHint);
    };
    
    HashTable<char*, char*, TemplateNameHashValue, TemplateNameEquals> m_htTemplates;

public:

    DBConverter();
    ~DBConverter();

    int Open (ConfigFile* pcfConfig);
    int Convert();

    inline int GetNumTablesConverted() { return m_iNumTablesConverted; }
};

#endif // !defined(AFX_DBCONVERTER_H__637CBFB0_1A75_40EC_8CED_FDECCCD15964__INCLUDED_)
