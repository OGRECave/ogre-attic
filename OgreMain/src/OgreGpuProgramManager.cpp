/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreGpuProgramManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> GpuProgramManager* Singleton<GpuProgramManager>::ms_Singleton = 0;
    //---------------------------------------------------------------------------
    GpuProgram* GpuProgramManager::load(const String& filename, GpuProgramType gptype, 
        const String& syntaxCode, int priority)
    {
        GpuProgram* prg = create(filename, gptype, syntaxCode);
        ResourceManager::load(prg, priority);
        return prg;
    }
    //---------------------------------------------------------------------------
	GpuProgram* GpuProgramManager::load(const String& name, const String& code, 
        GpuProgramType gptype, const String& syntaxCode, int priority)
    {
        GpuProgram* prg = create(name, gptype, syntaxCode);
        prg->setSource(code);
        ResourceManager::load(prg, priority);
        return prg;
    }
    //---------------------------------------------------------------------------
    GpuProgramManager& GpuProgramManager::getSingleton(void)
    {
        return Singleton<GpuProgramManager>::getSingleton();
    }
    //---------------------------------------------------------------------------
	GpuProgram* GpuProgramManager::createProgram(const String& filename, GpuProgramType gptype, 
        const String& syntaxCode, int priority)
    {
        GpuProgram* prg = create(filename, gptype, syntaxCode);
        ResourceManager::add(prg);
        return prg;
    }
    //---------------------------------------------------------------------------
	GpuProgram* GpuProgramManager::createProgram(const String& name, const String& code, 
        GpuProgramType gptype, const String& syntaxCode, int priority)
    {
        GpuProgram* prg = create(name, gptype, syntaxCode);
        prg->setSource(code);
        ResourceManager::add(prg);
        return prg;
    }
    //---------------------------------------------------------------------------
    bool GpuProgramManager::isSyntaxSupported(const String& syntaxCode) const
    {
        if (std::find(mSyntaxCodes.begin(), mSyntaxCodes.end(), syntaxCode) != mSyntaxCodes.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

}
