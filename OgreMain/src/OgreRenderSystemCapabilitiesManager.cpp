/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreMaterialManager.h"


#include "OgreStringVector.h"
#include "OgreLogManager.h"
#include "OgreArchive.h"
#include "OgreStringConverter.h"



#include "OgreException.h"
#include "OgreRenderSystemCapabilitiesManager.h"
#include "OgreRenderSystemCapabilitiesSerializer.h"



namespace Ogre {

    //-----------------------------------------------------------------------
    template<> RenderSystemCapabilitiesManager* Singleton<RenderSystemCapabilitiesManager>::ms_Singleton = 0;
    RenderSystemCapabilitiesManager* RenderSystemCapabilitiesManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    RenderSystemCapabilitiesManager& RenderSystemCapabilitiesManager::getSingleton(void)
    {
        assert( ms_Singleton );  return ( *ms_Singleton );
    }

    //-----------------------------------------------------------------------
    RenderSystemCapabilitiesManager::RenderSystemCapabilitiesManager()
    {
        mSerializer = new RenderSystemCapabilitiesSerializer();

		// Scripting is supported by this manager
		mScriptPatterns.push_back("*.rendercaps");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);

		// Resource type
		mResourceType = "RenderSystemCapabilities";

		// Register with resource group manager
		ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }
    //-----------------------------------------------------------------------
    RenderSystemCapabilitiesManager::~RenderSystemCapabilitiesManager()
    {
	    // Resources cleared by superclass
		// Unregister with resource group manager
		ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
		ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);

    }
	//-----------------------------------------------------------------------
	Resource* RenderSystemCapabilitiesManager::createImpl(const String& name, ResourceHandle handle,
		const String& group, bool isManual, ManualResourceLoader* loader,
        const NameValuePairList* params)
	{
		return new RenderSystemCapabilities(this, name, handle, group, isManual, loader);
	}

    //-----------------------------------------------------------------------
    void RenderSystemCapabilitiesManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
        // Delegate to serializer
        mSerializer->parseScript(stream, groupName);

    }
}





