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

#define OSAL_BUILD
#include "IObject.h"
#undef OSAL_BUILD

const Uuid IID_IObject = { 0x6538a8c9, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };

EXPORT bool operator== (const Uuid& uLeft, const Uuid& uRight) {
    return memcmp (&uLeft, &uRight, sizeof (Uuid)) == 0;
}