/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2004 The OGRE Team
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
// Ogre includes
#include "OgreStableHeaders.h"

#include "OgreResource.h"
#include "OgreResourceManager.h"
#include "OgreLogManager.h"

namespace Ogre 
{
	//-----------------------------------------------------------------------
	Resource::Resource(ResourceManager* creator, const String& name, ResourceHandle handle,
		const String& group, bool isManual, ManualResourceLoader* loader)
		: mCreator(creator), mName(name), mGroup(group), mHandle(handle), 
		mIsLoaded(false), mSize(0), mIsManual(isManual), mLoader(loader)
	{
	}
	//-----------------------------------------------------------------------
	Resource::~Resource() 
	{ 
		unload(); 
	}
	//-----------------------------------------------------------------------
	void Resource::load(void)
	{
		if (!mIsLoaded)
		{
			if (mIsManual)
			{
				// Load from manual loader
				if (mLoader)
				{
					mLoader->loadResource(this);
				}
				else
				{
					// Warn that this resource is not reloadable
					LogManager::getSingleton().logMessage(
						"WARNING: " + mCreator->getResourceType() + 
						" instance '" + mName + "' was defined as manually "
						"loaded, but no manual loader was provided. This Resource "
						"will be lost if it has to be reloaded.");
				}
			}
			else
			{
				loadImpl();
			}
			// Calculate resource size
			mSize = calculateSize();
			// Now loaded
			mIsLoaded = true;
			// Notify manager
			mCreator->_notifyResourceLoaded(this);
		}

	}
	//-----------------------------------------------------------------------
	void Resource::unload(void) 
	{ 
		if (mIsLoaded)
		{
			unloadImpl();
			mIsLoaded = false;
			// Notify manager
			mCreator->_notifyResourceUnloaded(this);
		}
	}
	//-----------------------------------------------------------------------
	void Resource::reload(void) 
	{ 
		if (mIsLoaded)
		{
			unload();
			load();
		}
	}
	//-----------------------------------------------------------------------
	void Resource::touch(void) 
	{
		mCreator->_notifyResourceTouched(this);
	}
	//-----------------------------------------------------------------------


}
