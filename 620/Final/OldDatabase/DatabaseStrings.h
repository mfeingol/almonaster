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

#if !defined(AFX_DATABASESTRINGS_H__05370B54_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)
#define AFX_DATABASESTRINGS_H__05370B54_2751_11D3_A0DC_0050047FE2E2__INCLUDED_

#define BACKUP_DIRECTORY "Backups"

#define TEMPLATE_DATA_FILE "Template.dat"
#define TABLE_DATA_FILE "Table.dat"

#define DATA_DIRECTORY "Data"

// Table row header
enum RowHeaderTag { VALID = 0xffffffff, INVALID = 0xfeedfeed, TERMINATOR = 0xdeadbeef};

struct ROW_HEADER {
    RowHeaderTag Tag;   // RowInfoTag
};

#endif