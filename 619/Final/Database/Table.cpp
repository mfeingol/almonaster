//
// Database.dll - A database library
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

#define DATABASE_BUILD
#include "Table.h"
#include "Index.h"
#include "DatabaseStrings.h"
#undef DATABASE_BUILD

#include <stdio.h>

#include "Osal/File.h"
#include "Osal/Algorithm.h"

// Min size is 4 KB
#define MIN_INITIAL_TABLE_SIZE (4 * 1024)
#define INITIAL_INSERTION_LENGTH_ASSUMPTION_SIZE (128)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Table::Table (const char* pszName, Template* pTemplate) {

	m_pszName = String::StrDup (pszName);

	pTemplate->AddRef();
	m_pTemplate = pTemplate;

	unsigned int iNumIndices = pTemplate->TemplateData.NumIndexes;

	if (iNumIndices > 0) {
		
		m_pIndex = new Index [iNumIndices];

		for (unsigned int i = 0; i < iNumIndices; i ++) {
			m_pIndex[i].Initialize (
				pTemplate->TemplateData.IndexColumn[i], 
				pTemplate->TemplateData.NumColumns,
				(pTemplate->TemplateData.Type)[pTemplate->TemplateData.IndexColumn[i]], 
				true, 
				NULL
				);
		}

	} else {
		m_pIndex = NULL;
	}

	m_bDelete = false;
	m_bLoaded = false;
	m_iNumRefs = 1;

	m_pszFileName = NULL;

	m_iTerminatorRowKey = 0;
	m_iNumRows = 0;

	m_stAddressSpace = 0;
	m_ppAddress = NULL;
}


Table::~Table() {

	if (m_pIndex != NULL) {
		delete [] m_pIndex;
	}

	m_mmfFile.Flush();
	m_mmfFile.Close();

	if (m_bDelete) {

		if (m_bLoaded) {

			m_pTemplate->DecrementTableSizeOnDisk (m_mmfFile.GetSize());
			m_pTemplate->DecrementNumLoadedRows (m_iNumRows);

		} else {

			size_t stSize;
			if (m_pszName != NULL && File::GetFileSize (m_pszFileName, &stSize) == OK) {
				m_pTemplate->DecrementTableSizeOnDisk (stSize);
			}
		}

		// Delete the table's directory
		char pszTableDir [OS::MaxFileNameLength];
		sprintf (pszTableDir, "%s/" DATA_DIRECTORY "/%s/%s", m_pTemplate->GetDirectory(), m_pTemplate->GetName(), m_pszName);

		int iErrCode = File::DeleteDirectory (pszTableDir);
		Assert (iErrCode == OK);
	}

	if (m_pTemplate != NULL) {
		m_pTemplate->Release();
	}

	if (m_pszName != NULL) {
		OS::HeapFree (m_pszName);
	}

	if (m_ppAddress != NULL) {
		delete [] m_ppAddress;
	}

	if (m_pszFileName != NULL) {
		OS::HeapFree (m_pszFileName);
	}
}

Table* Table::CreateInstance (const char* pszName, Template* pTemplate) {

	return new Table (pszName, pTemplate);
}

int Table::Flush() {
	return m_mmfFile.Flush();
}

int Table::Reload (const char* pszFileName) {

	return ReloadInternal (pszFileName, (m_pTemplate->GetOptions() & DELAY_TABLE_LOADS) != 0);
}

int Table::ReloadInternal (const char* pszFileName, bool bDelayLoad) {

	if (bDelayLoad) {
		Assert (m_pszFileName == NULL);
		m_pszFileName = String::StrDup (pszFileName);
		return OK;
	}

	unsigned int i;

	// Reload the map
	
	int iErrCode = m_mmfFile.OpenExisting (pszFileName);
	if (iErrCode != OK) {
		return iErrCode;
	}

	// Scan through table until terminator row is hit
	char* pRow = (char*) m_mmfFile.GetAddress();

	// Fix up indices
	for (i = 0; i < NumIndexCols; i ++) {
		m_pIndex[i].SetBaseAddress (pRow);
	}

	// Allocate initial address row space
	if (VariableLengthRows) {
		if (IsOneRow) {
			m_stAddressSpace = 2;
			m_ppAddress = new void* [2];
			m_ppAddress[0] = NULL;
		} else {
			m_stAddressSpace = 10;
			m_ppAddress = new void* [10];
			m_ppAddress[0] = NULL;
		}
	}

	bool bSeeking = true;
	unsigned int iKey = 0;

	size_t stRowSize = RowSize;

	void* pEndOfFile = EndOfFile, ** ppTemp;

	while (pRow < pEndOfFile) {

		// Read a row
		switch (((ROW_HEADER*) pRow)->Tag) {

		case INVALID:

			// Add to fragmentation queue
			m_fqFragQueue.Push (iKey);
			break;

		case VALID:
			
			// Add to indices
			for (i = 0; i < NumIndexCols; i ++) {
				m_pIndex[i].InsertRow (iKey, (char*) pRow + Offset (m_pIndex[i].GetColumn()));
			}

			m_iNumRows ++;

			break;
		
		case TERMINATOR:

			// Exit loop
			bSeeking = false;
			break;

		default:

			// Damaged table, terminate it now
			Assert (false);

			((ROW_HEADER*) pRow)->Tag = TERMINATOR;
			pRow = (char*) pEndOfFile;
			
			// Exit loop
			goto End;
		}

		if (VariableLengthRows) {
			
			if (iKey == m_stAddressSpace) {
				m_stAddressSpace *= 2;
				ppTemp = new void* [m_stAddressSpace];
				memcpy (ppTemp, m_ppAddress, iKey * sizeof (void*));
				delete [] m_ppAddress;
				m_ppAddress = ppTemp;
			}
			m_ppAddress[iKey] = (char*) pRow - (size_t) m_mmfFile.GetAddress();

			pRow = (char*) GetNextVariableLengthRow (pRow);
		} else {
			pRow += stRowSize;
		}
		
		if (!bSeeking) {
			break;
		}

		iKey ++;
	}

	// If for some reason we reached the end of the file without a terminator, return a warning 
	// and terminate with a new row
	if (bSeeking) {

		size_t stOldSize = m_mmfFile.GetSize();
		size_t stNewSize = max (FileSize * 3, FileSize + RowSize * 10);

		iErrCode = m_mmfFile.Resize (stNewSize);
		if (iErrCode != OK) {

			Assert (false);

			// Give up...
			m_fqFragQueue.Clear();
			m_mmfFile.Close();

			return iErrCode;
		}

		for (i = 0; i < NumIndexCols; i ++) {
			m_pIndex[i].SetBaseAddress (m_mmfFile.GetAddress());
		}

		RowHeader (iKey)->Tag = TERMINATOR;
		iErrCode = WARNING;

		m_pTemplate->IncrementTableSizeOnDisk (stNewSize - stOldSize);
	}

End:

	m_iTerminatorRowKey = iKey;

	// Make sure a OneRow table doesn't have more than one row
	if (IsOneRow && m_iTerminatorRowKey > 1) {
		
		m_fqFragQueue.Clear();
		m_mmfFile.Close();
		
		m_iTerminatorRowKey = 0;

		return ERROR_TABLE_HAS_MORE_THAN_ONE_ROW;
	}

	m_bLoaded = true;

	m_pTemplate->IncrementNumLoadedRows (m_iNumRows);

	return OK;
}


int Table::Create (const char* pszDirName) {

	// Create directory	
	int iErrCode;
	if (!File::DoesDirectoryExist (pszDirName) && File::CreateDirectory (pszDirName) != OK) {
		return ERROR_COULD_NOT_CREATE_TABLE_DIRECTORY;
	}

	char pszTableDataFile [OS::MaxFileNameLength];

	sprintf (pszTableDataFile, "%s" TABLE_DATA_FILE, pszDirName);
	if (File::DoesFileExist (pszTableDataFile)) {
		return ERROR_TABLE_ALREADY_EXISTS;
	}

	// Decide the size of the memmap
	size_t stSize;
	if (IsOneRow) {

		// Two rows is all we need
		stSize = RowSize * 2 + NumInitialInsertionLengthRows * INITIAL_INSERTION_LENGTH_ASSUMPTION_SIZE;

		if (VariableLengthRows) {
			m_stAddressSpace = 2;
			m_ppAddress = new void* [2];
			m_ppAddress[0] = NULL;
		}
	
	} else {

		// Start with at least space for 10 rows
		stSize = RowSize * 10 + NumInitialInsertionLengthRows * INITIAL_INSERTION_LENGTH_ASSUMPTION_SIZE;

		// Round up the initial size to the closest multiple of 2, for row sizes < 1MB
		if (stSize < MIN_INITIAL_TABLE_SIZE) {
			stSize = MIN_INITIAL_TABLE_SIZE;
		}

		if (VariableLengthRows) {
			m_stAddressSpace = 10;
			m_ppAddress = new void* [10];
			m_ppAddress[0] = NULL;
		}
	}
	
	// Align file size on 8 byte boundary
	stSize = ALIGN (stSize, 8);

	iErrCode = m_mmfFile.OpenNew (pszTableDataFile, stSize);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	// Fix up indices	
	for (unsigned int i = 0; i < NumIndexCols; i ++) {
		m_pIndex[i].SetBaseAddress (m_mmfFile.GetAddress());
	}

	// Set first row as terminator row
	((ROW_HEADER*) m_mmfFile.GetAddress())->Tag = TERMINATOR;

	m_iTerminatorRowKey = 0;

	m_bLoaded = true;

	return OK;
}

ROW_HEADER* Table::RowHeader (unsigned int iKey) {

	if (VariableLengthRows) {
		return (ROW_HEADER*) ((char*) m_ppAddress[iKey] + (size_t) m_mmfFile.GetAddress());
	} else {
		return (ROW_HEADER*) ((char*) m_mmfFile.GetAddress() + iKey * RowSize);
	}
}

void* Table::Data (unsigned int iKey, unsigned int iColumn) {

	if (VariableLengthRows) {

		char* pBase = (char*) m_ppAddress[iKey] + (size_t) m_mmfFile.GetAddress();
		
		unsigned int i;
		size_t stOffset = sizeof (ROW_HEADER), stSize;
		for (i = 0; i < iColumn; i ++) {

			stSize = Size (i);

			if (stSize == INITIAL_INSERTION_LENGTH) {
				stOffset += *((size_t*) (pBase + stOffset));
			} else {
				stOffset += stSize;
			}
		}

		pBase += stOffset;
		if (Size (iColumn) == INITIAL_INSERTION_LENGTH) {
			pBase += sizeof (size_t);
		}

		return pBase;

	} else {
		return (char*) m_mmfFile.GetAddress() + iKey * RowSize + Offset (iColumn);
	}
}

int Table::Backup (const char* pszBackupDir) {

	char pszBackup [OS::MaxFileNameLength];

	sprintf (pszBackup, "%s/%s/", pszBackupDir, m_pTemplate->GetName());

	// Best effort, since it might exist already
	File::CreateDirectory (pszBackup);

	// This has to succeed
	strcat (pszBackup, m_pszName);
	int iErrCode = File::CreateDirectory (pszBackup);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	if (m_bLoaded) {
		
		// Copy data file to data file in memory
		MemoryMappedFile mmfCopy;
		size_t stSize = m_mmfFile.GetSize();
		
		strcat (pszBackup, "/" TABLE_DATA_FILE);
		
		iErrCode = mmfCopy.OpenNew (pszBackup, stSize);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
		
		memcpy (mmfCopy.GetAddress(), m_mmfFile.GetAddress(), stSize);
		
		mmfCopy.Close();

	} else {

		Assert (m_pszFileName != NULL);

		strcat (pszBackup, "/" TABLE_DATA_FILE);

		// Use file system copy
		iErrCode = File::CopyFile (m_pszFileName, pszBackup);
	}

	return iErrCode;
}

void* Table::GetNextVariableLengthRow (void* pPrevRow) {
	
	unsigned int i;
	size_t stOffset = sizeof (ROW_HEADER), stSize;

	for (i = 0; i < NumberOfColumns; i ++) {

		stSize = Size (i);

		if (stSize == INITIAL_INSERTION_LENGTH) {
			stOffset += *((size_t*) ((char*) pPrevRow + stOffset));
		} else {
			stOffset += stSize;
		}
	}
	
	return (char*) pPrevRow + stOffset;
}

int Table::GetTemplate (ITemplate** ppTemplate) {
	
	*ppTemplate = m_pTemplate;
	m_pTemplate->AddRef();

	return OK;
}