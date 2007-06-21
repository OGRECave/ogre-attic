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
#ifndef __RENDERSYSTEMCAPABILITIESMANAGER_H__
#define __RENDERSYSTEMCAPABILITIESMANAGER_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreResourceManager.h"
#include "OgreStringVector.h"


//#include "OgreRenderSystemCapabilitiesSerializer.h"



#if OGRE_THREAD_SUPPORT
// boost::thread_specific_ptr has 'new' in header but delete in lib
// so if we use our memory manager it reports leaks incorrectly
#	include "OgreNoMemoryMacros.h"
#	include <boost/thread/tss.hpp>
#	include "OgreMemoryMacros.h"
#endif

namespace Ogre {


    /** Class for managing RenderSystemCapabilities database for Ogre.
        @remarks
           RenderSystemCapabilities can not loaded like other resource be because they do not contain any \
           data besides the contents of .rendercaps files, which are "loaded" by parseScript
    */
    class _OgreExport RenderSystemCapabilitiesManager : public ResourceManager, public Singleton<RenderSystemCapabilitiesManager>
    {

    protected:
		/// Overridden from ResourceManager
		Resource* createImpl(const String& name, ResourceHandle handle,
			const String& group, bool isManual, ManualResourceLoader* loader,
            const NameValuePairList* params);

        RenderSystemCapabilitiesSerializer* mSerializer;

    public:

        /** Default constructor.
        */
        RenderSystemCapabilitiesManager();

        /** Default destructor.
        */
        virtual ~RenderSystemCapabilitiesManager();


		/** @see ScriptLoader::parseScript
        */
        void parseScript(DataStreamPtr& stream, const String& groupName);


        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static RenderSystemCapabilitiesManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static RenderSystemCapabilitiesManager* getSingletonPtr(void);

    };

}

#endif