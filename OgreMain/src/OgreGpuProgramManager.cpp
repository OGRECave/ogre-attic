/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> GpuProgramManager* Singleton<GpuProgramManager>::ms_Singleton = 0;
    template<> GpuProgramManager* Singleton<GpuProgramManager>::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    template<> GpuProgramManager& Singleton<GpuProgramManager>::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //---------------------------------------------------------------------------
    GpuProgram* GpuProgramManager::load(const String& name, const String& filename, 
		GpuProgramType gptype, const String& syntaxCode, int priority)
    {
        GpuProgram* prg = create(name, gptype, syntaxCode);
		prg->setSourceFile(filename);
        ResourceManager::load(prg, priority);
        return prg;
    }
    //---------------------------------------------------------------------------
	GpuProgram* GpuProgramManager::loadFromString(const String& name, const String& code, 
        GpuProgramType gptype, const String& syntaxCode, int priority)
    {
        GpuProgram* prg = create(name, gptype, syntaxCode);
        prg->setSource(code);
        ResourceManager::load(prg, priority);
        return prg;
    }
    //---------------------------------------------------------------------------
	GpuProgram* GpuProgramManager::createProgram(const String& name, const String& filename, 
		GpuProgramType gptype, const String& syntaxCode, int priority)
    {
        GpuProgram* prg = create(name, gptype, syntaxCode);
		prg->setSourceFile(filename);
        ResourceManager::add(prg);
        return prg;
    }
    //---------------------------------------------------------------------------
	GpuProgram* GpuProgramManager::createProgramFromString(const String& name, 
		const String& code, GpuProgramType gptype, const String& syntaxCode, int priority)
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
    //---------------------------------------------------------------------------
    Resource* GpuProgramManager::getByName(const String& name, bool preferHighLevelPrograms)
    {
        Resource* ret;
        if (preferHighLevelPrograms)
        {
            ret = HighLevelGpuProgramManager::getSingleton().getByName(name);
            if (ret)
                return ret;
        }
        return ResourceManager::getByName(name);
    }

}
