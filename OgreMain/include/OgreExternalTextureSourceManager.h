/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef _OgreExternalTextureSourceManager_H
#define _OgreExternalTextureSourceManager_H

/***************************************************************************
OgreExternalTextureSourceManager.h  -  
	Handles the registering / unregistering of texture modifier plugins

-------------------
date                 : Jan 1 2004
email                : pjcast@yahoo.com
***************************************************************************/
#include "OgreSingleton.h"
#include "OgreString.h"
#include "OgreExternalTextureSource.h"

#include <map>

namespace Ogre
{
	/** 
	Singleton Class which handles the registering and control of texture plugins. The plugins
	will be mostly controlled via a string interface. */
	class _OgreExport ExternalTextureSourceManager : public Singleton<ExternalTextureSourceManager>
	{
	public:
		/** Constructor */
		ExternalTextureSourceManager();
		/** Destructor */
		~ExternalTextureSourceManager();

		/** ie. "video", "effect", "generic", etc.. */
		void SetCurrentPlugIn( String sTexturePlugInType );

		/** Returns currently selected plugin, may be null if none selected */
		ExternalTextureSource* getCurrentPlugIn() { return mpCurrExternalTextureSource; }
	
		/** Destroys a texture created by one of the registered controllers */
		void DestroyAdvancedTexture( String sTextureName );

		/** Returns the VideoSystem which reistered itself with a specific name ("wideo"), or null if none */
		ExternalTextureSource* getExternalTextureSource( String sTexturePlugInType );

		/** Called from VideoSystem plugin to register itself */
		void setExternalTextureSource( String sTexturePlugInType, ExternalTextureSource* pTextureSystem );

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
        static ExternalTextureSourceManager& getSingleton(void);

	protected:
		//The current texture controller selected
		ExternalTextureSource* mpCurrExternalTextureSource;
		//True if we are playing a movie via one of the two helper funcions
        // Collection of loaded texture System PlugIns, keyed by Video System type
        typedef std::map< String, ExternalTextureSource*> TextureSystemList;
        TextureSystemList mTextureSystems;
	};
} 
#endif
