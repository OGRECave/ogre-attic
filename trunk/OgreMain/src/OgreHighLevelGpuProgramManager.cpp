/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre {

	//-----------------------------------------------------------------------
	template<> HighLevelGpuProgramManager* 
	Singleton<HighLevelGpuProgramManager>::ms_Singleton = 0;
	//-----------------------------------------------------------------------
	HighLevelGpuProgramManager::HighLevelGpuProgramManager()
	{
	}
	//-----------------------------------------------------------------------
	HighLevelGpuProgramManager::~HighLevelGpuProgramManager()
	{
	}
    //---------------------------------------------------------------------------
	void HighLevelGpuProgramManager::addFactory(HighLevelGpuProgramFactory* factory)
	{
		// deliberately allow later plugins to override earlier ones
		mFactories[factory->getLanguage()] = factory;
	}
    //---------------------------------------------------------------------------
	HighLevelGpuProgramFactory* HighLevelGpuProgramManager::getFactory(const String& language)
	{
		FactoryMap::iterator i = mFactories.find(language);

		if (i == mFactories.end())
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find factory for language '" + language + "'",
				"HighLevelGpuProgramManager::getFactory");
		}
		return i->second;
	}
    //---------------------------------------------------------------------------
    HighLevelGpuProgram* HighLevelGpuProgramManager::createProgram(
			const String& name, const String& language, GpuProgramType gptype, 
            int priority)
    {
        HighLevelGpuProgram* ret = getFactory(language)->create(name, gptype);
        add(ret);
        return ret;
    }
    //---------------------------------------------------------------------------
    HighLevelGpuProgramManager& HighLevelGpuProgramManager::getSingleton(void)
    {
        return Singleton<HighLevelGpuProgramManager>::getSingleton();
    }
}
