/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#ifndef __SkeletonManager_H__
#define __SkeletonManager_H__

#include "OgrePrerequisites.h"

#include "OgreResourceManager.h"
#include "OgreSingleton.h"

namespace Ogre {

    /** Handles the management of skeleton resources.
        @remarks
            This class deals with the runtime management of
            skeleton data; like other resource managers it handles
            the creation of resources (in this case skeleton data),
            working within a fixed memory budget.
    */
    class _OgreExport SkeletonManager: public ResourceManager, public Singleton<SkeletonManager>
    {
    public:
        /// Constructor
        SkeletonManager();

        /** Loads a skeleton from a file, making it available for use.
            @note
                If the skeleton has already been loaded, the existing instance
                will be returned.
            @remarks
                Ogre loads skeleton files from it's own proprietary
                format called .skeleton. This is because having a single file
                format is better for runtime performance, and we also have
                control over pre-processed data.
        */
        Skeleton* load( const String& filename, int priority = 1);

        /** Creates a Skeleton resource.
            @note
                Mainly used internally.
        */
        Resource* create( const String& name);

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton implementation is in a .h file,
                which means it gets compiled into anybody who includes it. This is needed for the Singleton
                template to work, but we actually only want it compiled into the implementation of the
                class based on the Singleton, not all of them. If we don't change this, we get link errors
                when trying to use the Singleton-based class from an outside dll.
            @par
                This method just delegates to the template version anyway, but the implementation stays in this
                single compilation unit, preventing link errors.
        */
        static SkeletonManager& getSingleton(void);


    };


}


#endif
