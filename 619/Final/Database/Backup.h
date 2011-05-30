// Backup.h: interface for the Backup class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BACKUP_H__B530F1C3_5133_11D3_A19B_0050047FE2E2__INCLUDED_)
#define AFX_BACKUP_H__B530F1C3_5133_11D3_A19B_0050047FE2E2__INCLUDED_

#define DATABASE_BUILD
#include "Database.h"
#undef DATABASE_BUILD

class Database;

class Backup : public IDatabaseBackup {
private:

	Backup (int iDay, int iMonth, int iYear, int iVersion);

	int m_iYear;
	int m_iMonth;
	int m_iDay;
	int m_iVersion;

public:

	static Backup* CreateInstance (int iDay, int iMonth, int iYear, int iVersion);

	// IDatabaseBackup
	IMPLEMENT_INTERFACE (IDatabaseBackup);

	int GetDate (int* piDay, int* piMonth, int* piYear, int* piVersion);
};


class BackupEnumerator : public IDatabaseBackupEnumerator {
private:

	BackupEnumerator (IDatabaseBackup** ppBackupArray, unsigned int iNumBackups);
	~BackupEnumerator();

	IDatabaseBackup** m_ppBackupArray;
	unsigned int m_iNumBackups;


public:

	static BackupEnumerator* CreateInstance (IDatabaseBackup** ppBackupArray, unsigned int iNumBackups);

	// IDatabaseBackup
	IMPLEMENT_INTERFACE (IDatabaseBackupEnumerator);

	unsigned int GetNumBackups();
	IDatabaseBackup** GetBackups();
};

#endif // !defined(AFX_BACKUP_H__B530F1C3_5133_11D3_A19B_0050047FE2E2__INCLUDED_)