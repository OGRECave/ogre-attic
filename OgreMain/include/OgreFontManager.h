/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/

#ifndef _FontManager_H__
#define _FontManager_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreResourceManager.h"

namespace Ogre
{
    /** Manages Font resources, parsing .fontdef files and generally organising them.*/
    class _OgreExport FontManager : public ResourceManager, public Singleton< FontManager >
    {
    public:

        /** Creates a new font. 
        @remarks
            Creates a new Font, but does not load it yet. You must populate all the
            relevant fields and then call load() yourself.
            See ResourceManager for more info. 
        */
        Resource * create(const String& name);

        /** Parse a .fontdef script passed in as a chunk. */
        void parseScript( DataChunk& chunk );
        /** Parses all .fontdef scripts available in all resource locations. */
        void parseAllSources( const String& extension = ".fontdef" );
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
        static FontManager& getSingleton(void);
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
        static FontManager* getSingletonPtr(void);

    protected:

        /// Internal methods
        void parseAttribute(const String& line, Font* pFont);

        void logBadAttrib(const String& line, Font* pFont);

    };
}

#endif
