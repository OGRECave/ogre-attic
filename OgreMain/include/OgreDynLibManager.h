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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#ifndef __DynLibManager_H__
#define __DynLibManager_H__

#include "OgrePrerequisites.h"

#include "OgreResourceManager.h"
#include "OgreSingleton.h"

namespace Ogre {
    /** Manager for Dynamic-loading Libraries.
        @remarks
            This manager keeps a track of all the open dynamic-loading
            libraries, opens them and returns references to already-open
            libraries.
        @author
            Adrian Cearnãu (cearny@cearny.ro)
        @since
            27 January 2002
        @see
            ResourceManager
    */
    class _OgreExport DynLibManager: public ResourceManager, public Singleton<DynLibManager>
    {
    public:
        /** Default constructor.
            @note
                <br>Should never be called as the singleton is automatically
                created during the creation of the Root object.
            @see
                Root::Root
        */
        DynLibManager() {}

        /** Default destructor.
            @see
                Root::~Root
        */
        virtual ~DynLibManager();

        /** Loads the passed library.
            @param
                filename The name of the library. The extension can be ommitted
            @param
                priority The priority of the library. Not used
            @author
                Adrian Cearnãu (cearny at cearny dot ro)
            @since
                27 January 2002
        */
        DynLib* load( const String& filename, int priority = 1);

        /** Creates a DynLib resource.
            @note
                <br>Mainly used internally.
        */
        Resource* create( const String& name);

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton
                implementation is in a .h file, which means it gets compiled
                into anybody who includes it. This is needed for the
                Singleton template to work, but we actually only want it
                compiled into the implementation of the class based on the
                Singleton, not all of them. If we don't change this, we get
                link errors when trying to use the Singleton-based class from
                an outside DLL.
            @par
                This method just delegates to the template version anyway,
                but the implementation stays in this single compilation unit,
                preventing link errors.
        */
        static DynLibManager& getSingleton(void);
    };
}

#endif
