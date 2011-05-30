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

#if !defined(AFX_DEFAULTMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
#define AFX_DEFAULTMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_

#include "BaseMapGenerator.h"

class DefaultMapGenerator : public BaseMapGenerator {
private:

    // Methods
    DefaultMapGenerator (IGameEngine* pGameEngine);

    virtual int CreatePlanetChains();

    int AllocateDefaultPlanetData();

public:

    static IMapGenerator* CreateInstance(IGameEngine* pGameEngine);
};

#endif // !defined(AFX_DEFAULTMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
