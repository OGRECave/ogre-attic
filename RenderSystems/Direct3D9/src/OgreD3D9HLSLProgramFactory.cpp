/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include "OgreD3D9HLSLProgramFactory.h"
#include "OgreString.h"
#include "OgreD3D9HLSLProgram.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    String sLanguageName = "hlsl";
    //-----------------------------------------------------------------------
    D3D9HLSLProgramFactory::D3D9HLSLProgramFactory()
    {
    }
    //-----------------------------------------------------------------------
    D3D9HLSLProgramFactory::~D3D9HLSLProgramFactory()
    {
    }
    //-----------------------------------------------------------------------
    const String& D3D9HLSLProgramFactory::getLanguage(void)
    {
        return sLanguageName;
    }
    //-----------------------------------------------------------------------
    HighLevelGpuProgram* D3D9HLSLProgramFactory::create(const String& name, GpuProgramType gptype)
    {
        return new D3D9HLSLProgram(name, gptype, sLanguageName);
    }
    //-----------------------------------------------------------------------
	void D3D9HLSLProgramFactory::destroy(HighLevelGpuProgram* prog)
    {
        delete prog;
    }
    //-----------------------------------------------------------------------

}
